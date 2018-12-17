#pragma once
///For getting working directory path
#ifdef _WIN32
#include <direct.h>
#define GetCurrentDir _getcwd
#else
#include <unistd.h>
#define GetCurrentDir getcwd
#endif

#define PROFILING
#define BBAccel
//#define ONE_FRAME
//#define FINAL_RENDER

#define HIT_BIAS 0.0005f // to prevent shadow acne

// Preprocessor directive for converting colors from floats to unsigned and vice-versa.
#define F2U(float_x) (uint8_t)round(float_x * 255.0f)
#define F32vec2U8vec(float_vec) glm::u8vec3(F2U(float_vec[0]),F2U(float_vec[1]),F2U(float_vec[2]))
#define U2F(uint_x) (float)uint_x/255.0f
#define U8vec2F32vec(uint_vec) glm::f32vec3(U2F(uint_vec[0]),U2F(uint_vec[1]),U2F(uint_vec[2]))

#define WIDTH 600
#define HEIGHT 480

#define PROFILE

#ifndef M_PI
#define M_PI 3.14159265358979323846264f
#endif

#define TEXTURE_REPEAT

#ifndef FINAL_RENDER
#define SCREEN_SPACE_SUBSAMPLE 1
#define WIDTH 640.f
#define HEIGHT 480.f
#define MAX_DEPTH 3
#define SMOOTH_SHADING
#else
#define SCREEN_SPACE_SUBSAMPLE 1
#define WIDTH 1024.f
#define HEIGHT 768.f
#define MAX_DEPTH 3
#define SMOOTH_SHADING
#endif


//const std::string DEFAULT_MODEL = "example/sponza/sponza.obj";
const std::string DEFAULT_MODEL = "example/CornellBox/CornellBox-Mirror.obj";
//const std::string DEFAULT_MODEL = "example/bunny/bunny.obj";
//const std::string DEFAULT_MODEL = "example/f16/f16.obj";
//const std::string DEFAULT_MODEL = "example/suzanne/suzanne.obj";
//const std::string DEFAULT_MODEL = "example/cruiser/cruiser.obj";
//const std::string DEFAULT_MODEL = "example/armadillo/armadillo.ply";