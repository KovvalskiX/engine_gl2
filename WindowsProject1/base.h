#pragma once

#include <stdio.h>
#include <string>
#include <cassert>
#include <functional>
#include <map>
#include <memory>
#include <chrono>
#include <imgui.h>
#include <imgui_impl_sdl.h>
#include <imgui_impl_opengl2.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include <boost/signals2.hpp>
#include <random>
#include <sstream>

#ifndef imPushMatrix
    /// @brief Push matrix and set display-sized ortho 
    #define imPushMatrix() {glPushMatrix();glOrtho(0,io.DisplaySize.x,io.DisplaySize.y,0,-1,1);}    
#endif

class glImage {
    private:
        GLuint texture;
    public:
        std::string path;
        glImage(std::string path);
        ~glImage();
        GLuint operator()();
};

namespace uuid {
    static std::random_device              rd;
    static std::mt19937                    gen(rd());
    static std::uniform_int_distribution<> dis(0, 15);
    static std::uniform_int_distribution<> dis2(8, 11);

    inline std::string new_v4() {
        std::stringstream ss;
        int i;
        ss << std::hex;
        for (i = 0; i < 8; i++) {
            ss << dis(gen);
        }
        ss << "-";
        for (i = 0; i < 4; i++) {
            ss << dis(gen);
        }
        ss << "-4";
        for (i = 0; i < 3; i++) {
            ss << dis(gen);
        }
        ss << "-";
        ss << dis2(gen);
        for (i = 0; i < 3; i++) {
            ss << dis(gen);
        }
        ss << "-";
        for (i = 0; i < 12; i++) {
            ss << dis(gen);
        };
        return ss.str();
    }
}

template<typename T>
class vec_t {
    public:
    T x, y, z, w;
    vec_t();
    vec_t(T x, T y, T z, T w = 0);
    std::string str();
};

class vec {
    public:
    float x,y,z,w;
    vec();
    vec(float x, float y, float z, float w=0);
    std::string str();
};

typedef vec point;

class aabb {
    public:
    point c, hw;
    aabb(): c(0,0,0,0), hw(0,0,0,0) {}
    aabb(point center, point halfwidths);
    bool overlap(aabb* b);
};

class log {
    int level;
    void console_(std::string str, uint16_t limit);
    public:
    /* 
    * не заработало, sink не может быть инициализирован тем же типом блять
    * typedef void(log::* Sink)(std::string str, uint16_t limit, uint16_t level);
    * сделал без typedef, магия!
    * https://isocpp.org/wiki/faq/pointers-to-members#macro-for-ptr-to-memfn
    */
    void(log::*sink)(std::string str, uint16_t limit);
    log (uint16_t log_level);
    void info(std::string str);
    void warn(std::string str);
    void err(std::string str);
};

class world;
class ent {
    bool invoke_(void(*callable)(ent*)) {
        if(callable != nullptr) {
            callable(this);
            return true;
        }
        return false;
    }
    /// @brief appends ent to world's ents (map<string,ent>) and generates an uuid
    /// @param game (world*) where to append 
    void w_append_(world* game);
    void w_erase_();
    public:
    void(*on_delete)(ent*)=[](ent*){};
    void(*on_update)(ent*)=[](ent*){};
    void(*on_draw)(ent*)=[](ent*){};
    std::string e_uuid;
    aabb collider;
    world* w;
    
    ent(world* w, void(*on_create)(ent*) = [](ent*){});
    ~ent();
    void update();
    void draw();
};

class world : public log {
    public:
    std::map<std::string, std::unique_ptr<ent>> ents;
    world(uint16_t log_level) : log(log_level) {}
    ~world() {
        for (auto & [key, val] : ents) {
            val.release();
        }
        ents.clear();
    }
};



// /// @brief Basic game object. Every component is derived from this struct.
// class body {
//     public:
//     world* game;
//     std::vector<std::unique_ptr<body>>::iterator ent_iterator;
//     vec3f position;
//     body(world* w);
//     body(world* w, vec3f position);
//     body(world* w, float x, float y, float z);
//     ~body();
//     private:
//     void append_to_world_();
// };

// template<typename base>
// class AABBComponent : public base {
//     bool overlaps = false;
//     vec3f prevpos;
//     public:
//     aabb collider;
//     AABBComponent(aabb collider, base component) : collider(collider), base(component), prevpos(0.0, 0.0, 0.0) {}
//     bool overlap(AABBComponent<base>* b);
//     void draw(ImGuiIO& io);
//     void prevent();
// };
