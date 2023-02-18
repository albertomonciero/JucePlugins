#pragma once

#if 0

BEGIN_JUCE_MODULE_DECLARATION

      ID:               shared_modules
      vendor:           Alberto Monciero
      version:          0.0.1
      name:             shared_modules
      description:      Shared modules
      license:          GPL/Commercial
      dependencies:     juce_dsp

     END_JUCE_MODULE_DECLARATION

#endif

#include <juce_dsp/juce_dsp.h>

#include "Source/OnePoleFilter.h"
#include "Source/VariableDelayLine.h"
#include "Source/VariableDelayAllpass.h"
