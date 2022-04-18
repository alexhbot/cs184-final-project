#include "bsdf.h"

#include <algorithm>
#include <iostream>
#include <utility>

#include "application/visual_debugger.h"

using std::max;
using std::min;
using std::swap;

namespace CGL {

// Mirror BSDF //

Vector3D MirrorBSDF::f(const Vector3D wo, const Vector3D wi) {
  return Vector3D();
}

Vector3D MirrorBSDF::sample_f(const Vector3D wo, Vector3D* wi, double* pdf) {

  // TODO Project 3-2: Part 1
  // Implement MirrorBSDF
    *pdf = 1;
    reflect(wo, wi);
    return reflectance / abs_cos_theta(*wi);
}

void MirrorBSDF::render_debugger_node()
{
  if (ImGui::TreeNode(this, "Mirror BSDF"))
  {
    DragDouble3("Reflectance", &reflectance[0], 0.005);
    ImGui::TreePop();
  }
}

// Microfacet BSDF //

double MicrofacetBSDF::G(const Vector3D wo, const Vector3D wi) {
  return 1.0 / (1.0 + Lambda(wi) + Lambda(wo));
}

double MicrofacetBSDF::D(const Vector3D h) {
  // TODO Project 3-2: Part 2
  // Compute Beckmann normal distribution function (NDF) here.
  // You will need the roughness alpha.
    
    return exp(-1.0 * pow(sin_theta(h) / cos_theta(h), 2) / pow(alpha, 2)) / PI / pow(alpha, 2) / pow(cos_theta(h), 4);
}

Vector3D MicrofacetBSDF::F(const Vector3D wi) {
  // TODO Project 3-2: Part 2
  // Compute Fresnel term for reflection on dielectric-conductor interface.
  // You will need both eta and etaK, both of which are Vector3D.
    Vector3D Rs = ((eta * eta + k * k) - 2.0 * eta * cos_theta(wi) + pow(cos_theta(wi), 2)) / ((eta * eta + k * k) + 2.0 * eta * cos_theta(wi) + pow(cos_theta(wi), 2));
    Vector3D Rp = ((eta * eta + k * k) * pow(cos_theta(wi), 2) - 2.0 * eta * cos_theta(wi) + 1.0) / ((eta * eta + k * k) * pow(cos_theta(wi), 2) + 2.0 * eta * cos_theta(wi) + 1.0);
    return (Rs + Rp) / 2.0;
}

Vector3D MicrofacetBSDF::f(const Vector3D wo, const Vector3D wi) {
  // TODO Project 3-2: Part 2
  // Implement microfacet model here.
    if (wo.z <= 0 || wi.z <= 0) {
        return Vector3D();
    }
    Vector3D h = ((wo + wi) / (wo + wi).norm()).unit();
    return F(wi) * G(wo, wi) * D(h) / 4.0 / wo.z / wi.z;
}

Vector3D MicrofacetBSDF::sample_f(const Vector3D wo, Vector3D* wi, double* pdf) {
  // TODO Project 3-2: Part 2
  // *Importance* sample Beckmann normal distribution function (NDF) here.
  // Note: You should fill in the sampled direction *wi and the corresponding *pdf,
  //       and return the sampled BRDF value.
    Vector2D sample = sampler.get_sample();
    double r1 = sample[0];
    double r2 = sample[1];
    
    double theta_h = atan(sqrt(-1.0 * pow(alpha, 2) * log(1.0 - r1)));
    double phi_h = 2.0 * PI * r2;
    
    double p_theta = 2.0 * sin(theta_h) * exp(-1.0 * pow(tan(theta_h), 2) / pow(alpha, 2)) / pow(alpha, 2) / pow(cos(theta_h), 3);
    double p_phi = 0.5 / PI;
    
    Vector3D h = Vector3D(sin(theta_h) * cos(phi_h), sin(phi_h) * sin(theta_h), cos(theta_h)).unit();
    
    //begin
    Matrix3x3 o2w;
    make_coord_space(o2w, h);
    Matrix3x3 w2o = o2w.T();
    *wi = o2w * Vector3D(-(w2o * wo).x, -(w2o * wo).y, (w2o * wo).z);
    //end
    
    if (wi->z <= 0) {
        *pdf = 0;
        *wi = Vector3D();
        return Vector3D();
    }
    
    *pdf = p_theta * p_phi / sin(theta_h) / 4.0 / dot(*wi, h);

    //*wi = cosineHemisphereSampler.get_sample(pdf);
    
    return MicrofacetBSDF::f(wo, *wi);
}

void MicrofacetBSDF::render_debugger_node()
{
  if (ImGui::TreeNode(this, "Micofacet BSDF"))
  {
    DragDouble3("eta", &eta[0], 0.005);
    DragDouble3("K", &k[0], 0.005);
    DragDouble("alpha", &alpha, 0.005);
    ImGui::TreePop();
  }
}

// Refraction BSDF //

Vector3D RefractionBSDF::f(const Vector3D wo, const Vector3D wi) {
  return Vector3D();
}

Vector3D RefractionBSDF::sample_f(const Vector3D wo, Vector3D* wi, double* pdf) {
  // TODO Project 3-2: Part 1
  // Implement RefractionBSDF
    double eta;
    if (wo.z > 0) {
        // entering
        eta = 1.0 / ior;
    } else {
        // exiting
        eta = ior;
    }
    if (refract(wo, wi, ior)) {
        *pdf = 1;
        return transmittance / abs_cos_theta(*wi) / pow(eta, 2);
    }
    return Vector3D();
}

void RefractionBSDF::render_debugger_node()
{
  if (ImGui::TreeNode(this, "Refraction BSDF"))
  {
    DragDouble3("Transmittance", &transmittance[0], 0.005);
    DragDouble("ior", &ior, 0.005);
    ImGui::TreePop();
  }
}

// Glass BSDF //

Vector3D GlassBSDF::f(const Vector3D wo, const Vector3D wi) {
  return Vector3D();
}

Vector3D GlassBSDF::sample_f(const Vector3D wo, Vector3D* wi, double* pdf) {

  // TODO Project 3-2: Part 1
  // Compute Fresnel coefficient and either reflect or refract based on it.

  // compute Fresnel coefficient and use it as the probability of reflection
  // - Fundamentals of Computer Graphics page 305
    if (!refract(wo, wi, ior)) {
        reflect(wo, wi);
        *pdf = 1;
        return reflectance / abs_cos_theta(*wi);
    } else {
        double eta;
        if (wo.z > 0) {
            // entering
            eta = 1.0 / ior;
        } else {
            // exiting
            eta = ior;
        }
        double R0 = pow((1.0 - eta)/(1.0 + eta), 2);
        double R = R0 + (1.0 - R0) * pow(1.0 - abs_cos_theta(wo), 5);
        if (coin_flip(R)) {
            reflect(wo, wi);
            *pdf = R;
            return R * reflectance / abs_cos_theta(*wi);
        } else {
            refract(wo, wi, ior);
            *pdf = 1 - R;
            return (1 - R) * transmittance / abs_cos_theta(*wi) / pow(eta, 2);
        }
    }
}

void GlassBSDF::render_debugger_node()
{
  if (ImGui::TreeNode(this, "Refraction BSDF"))
  {
    DragDouble3("Reflectance", &reflectance[0], 0.005);
    DragDouble3("Transmittance", &transmittance[0], 0.005);
    DragDouble("ior", &ior, 0.005);
    ImGui::TreePop();
  }
}

void BSDF::reflect(const Vector3D wo, Vector3D* wi) {

  // TODO Project 3-2: Part 1
  // Implement reflection of wo about normal (0,0,1) and store result in wi.
    
    *wi = Vector3D(-wo.x, -wo.y, wo.z);

}

bool BSDF::refract(const Vector3D wo, Vector3D* wi, double ior) {

  // TODO Project 3-2: Part 1
  // Use Snell's Law to refract wo surface and store result ray in wi.
  // Return false if refraction does not occur due to total internal reflection
  // and true otherwise. When dot(wo,n) is positive, then wo corresponds to a
  // ray entering the surface through vacuum.
    
    double eta, sign;
    if (wo.z > 0) {
        // entering
        eta = 1.0 / ior;
        sign = -1.0;
    } else {
        // exiting
        eta = ior;
        sign = 1.0;
    }
    
    // total internal reflection
    double temp = 1.0 - pow(eta, 2) * (1.0 - pow(wo.z, 2));
    if (temp < 0) {
        return false;
    }
    
    *wi = Vector3D(-1.0 * eta * wo.x, -1.0 * eta * wo.y, sign * sqrt(temp));
    return true;
}

} // namespace CGL
