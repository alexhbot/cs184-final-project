#!/bin/sh
set -e
if test "$CONFIGURATION" = "Debug"; then :
  cd /Users/alexhao/desktop/CS184/cs184-final-project/xcode
  make -f /Users/alexhao/desktop/CS184/cs184-final-project/xcode/CMakeScripts/ReRunCMake.make
fi
if test "$CONFIGURATION" = "Release"; then :
  cd /Users/alexhao/desktop/CS184/cs184-final-project/xcode
  make -f /Users/alexhao/desktop/CS184/cs184-final-project/xcode/CMakeScripts/ReRunCMake.make
fi
if test "$CONFIGURATION" = "MinSizeRel"; then :
  cd /Users/alexhao/desktop/CS184/cs184-final-project/xcode
  make -f /Users/alexhao/desktop/CS184/cs184-final-project/xcode/CMakeScripts/ReRunCMake.make
fi
if test "$CONFIGURATION" = "RelWithDebInfo"; then :
  cd /Users/alexhao/desktop/CS184/cs184-final-project/xcode
  make -f /Users/alexhao/desktop/CS184/cs184-final-project/xcode/CMakeScripts/ReRunCMake.make
fi

