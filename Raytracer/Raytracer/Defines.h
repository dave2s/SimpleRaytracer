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
//#define ONE_FRAME

#define HIT_BIAS 0.0005f // to prevent shadow acne

// Preprocessor directive for converting colors from floats to unsigned and vice-versa.
#define F2U(float_x) (uint8_t)round(float_x * 255.0f)
#define F32vec2U8vec(float_vec) glm::u8vec3(F2U(float_vec[0]),F2U(float_vec[1]),F2U(float_vec[2]))
#define U2F(uint_x) (float)uint_x/255.0f
#define U8vec2F32vec(uint_vec) glm::f32vec3(U2F(uint_vec[0]),U2F(uint_vec[1]),U2F(uint_vec[2]))

#ifndef M_PI
#define M_PI 3.14159265358979323846264f
#endif

#define TEXTURE_REPEAT
//#define FINAL_RENDER
#ifndef FINAL_RENDER
#define SCREEN_SPACE_SUBSAMPLE 2
//#define BBAccel
#define BVH_ACCEL
//#define MSAA
#define WIDTH 320
#define HEIGHT 240
#define MAX_DEPTH 6
#define WAVE_SAMPLES 3
#define SMOOTH_SHADING
#define PROFILE
#else
#define SCREEN_SPACE_SUBSAMPLE 1
#define MSAA
#define BBAccel
#define WIDTH 1024
#define HEIGHT 768
#define MAX_DEPTH 3
#define WAVE_SAMPLES 10
#define SMOOTH_SHADING
#define PROFILE
#endif

#ifdef MSAA
const std::vector<std::vector<float>> msaa_sample_coords/*(2, std::vector< float >(2, 0.f))*/{ {0.f,0.f}, {-0.25f,+0.25f}, {+0.25f,+0.25f},{-0.25f,-0.25f},{+0.25f,-0.25f} };
#endif

// mark functions returning via reference
#define OUT

// convert  <0.f,1.f> to <0,255>
#define F2U(float_x) (uint8_t)round(float_x * 255.0f)
#define F32vec2U8vec(float_vec) glm::u8vec3(F2U(float_vec[0]),F2U(float_vec[1]),F2U(float_vec[2]))
#define U2F(uint_x) (float)uint_x/255.0f
#define U8vec2F32vec(uint_vec) glm::f32vec3(U2F(uint_vec[0]),U2F(uint_vec[1]),U2F(uint_vec[2]))


const float global_light_intensity = .1f;
const glm::f32vec3 const_sky_color = glm::f32vec3(U2F(160), U2F(217), U2F(255));

//const std::string DEFAULT_MODEL = "example/sponza2/sponza.obj";
//const std::string DEFAULT_MODEL = "example/CornellBox/CornellBox-Mirror.obj";
//const std::string DEFAULT_MODEL = "example/prism/prism.obj";
const std::string DEFAULT_MODEL = "example/prism/prism2.obj";
//const std::string DEFAULT_MODEL = "example/bunny/bunny.obj";
//const std::string DEFAULT_MODEL = "example/f16/f16.obj";
//const std::string DEFAULT_MODEL = "example/suzanne/suzanne.obj";
//const std::string DEFAULT_MODEL = "example/cruiser/cruiser.obj";
//const std::string DEFAULT_MODEL = "example/armadillo/armadillo.ply";