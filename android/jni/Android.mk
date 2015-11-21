LOCAL_PATH:= $(call my-dir)

# ------------------------------------------------------------------
# Static library
# ------------------------------------------------------------------

include $(CLEAR_VARS)

LOCAL_MODULE := libgammaengine

ifneq (,$(filter $(TARGET_ARCH), x86_64 arm64 arm64-v8 mips64))
	ARCH = 64
else
	ARCH = 32
endif

SRC := $(wildcard ../src/*.cpp)
SRC += $(wildcard ../backends/opengles20/src/*.cpp)
SRC += $(wildcard ../src/android/*.cpp)
SRC := $(filter-out ../src/Vector.cpp, $(SRC))
SRC := $(filter-out $(wildcard ../src/Physical*.cpp), $(SRC))
SRC := $(filter-out $(wildcard ../src/main*.cpp), $(SRC))

$(info $(SRC))

LOCAL_SRC_FILES = $(addprefix ../, $(SRC))

LOCAL_CPPFLAGS := -g2 -Os -Wall -Wno-unused -std=gnu++0x -std=c++11 -fPIC -I../ -I../include -I../include/android -I../backends/opengles20/include -DGE_LIB -DGE_ANDROID -DGE_STATIC_BACKEND=opengles20

LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)

# LOCAL_LDLIBS    := -lfreetype2-static -ljpeg9 -lpng -lz -lm -llog -landroid -lEGL -lGLESv2
LOCAL_WHOLE_STATIC_LIBRARIES := android_native_app_glue

LOCAL_C_INCLUDES += $(NDK_ROOT)/sources/cxx-stl/gnu-libstdc++/4.9/include
ifneq (,$(filter $(TARGET_ARCH), arm armeabi))
	LOCAL_C_INCLUDES += $(NDK_ROOT)/sources/cxx-stl/gnu-libstdc++/4.9/libs/armeabi/include
	LOCAL_LDLIBS += $(NDK_ROOT)/sources/cxx-stl/gnu-libstdc++/4.9/libs/armeabi/libgnustl_static.a
else ifneq (,$(filter $(TARGET_ARCH), arm64))
	LOCAL_C_INCLUDES += $(NDK_ROOT)/sources/cxx-stl/gnu-libstdc++/4.9/libs/arm64-v8a/include
	LOCAL_LDLIBS += $(NDK_ROOT)/sources/cxx-stl/gnu-libstdc++/4.9/libs/arm64-v8a/libgnustl_static.a
else
	LOCAL_C_INCLUDES += $(NDK_ROOT)/sources/cxx-stl/gnu-libstdc++/4.9/libs/$(TARGET_ARCH)/include
	LOCAL_LDLIBS += $(NDK_ROOT)/sources/cxx-stl/gnu-libstdc++/4.9/libs/$(TARGET_ARCH)/libgnustl_static.a
endif
$(warning $(LOCAL_C_INCLUDES))

include $(BUILD_STATIC_LIBRARY)
# include $(BUILD_SHARED_LIBRARY)

$(call import-module, android/native_app_glue)
