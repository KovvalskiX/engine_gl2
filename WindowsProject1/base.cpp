
// #include "imgui.h"
// #include "imgui_impl_sdl.h"
// #include "imgui_impl_opengl2.h"
// #include <SDL2/SDL.h>
// #include <SDL2/SDL_opengl.h>
// #include <SDL2/SDL_image.h>
#include "base.h"

GLuint glImage::operator()() {
    return texture;
}
glImage::glImage(std::string path) : path(path) {
    //Load the image from the file into SDL's surface representation
    SDL_Surface* surf = IMG_Load(path.c_str());
    if (surf==NULL) { //If failed, say why and don't continue loading the texture
        printf("Error: \"%s\"\n",SDL_GetError()); return;
    }
 
    //Determine the data format of the surface by seeing how SDL arranges a test pixel.  This probably only works
    //  correctly for little-endian machines.
    GLenum data_fmt;
    Uint8 test = SDL_MapRGB(surf->format, 0xAA,0xBB,0xCC)&0xFF;
    if      (test==0xAA) data_fmt=GL_RGB;
    else if (test==0xCC) data_fmt=0x80E0;//GL_BGR;
    else {
        printf("Error: \"Loaded surface was neither RGB or BGR!\""); return;
    }
 
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // fixed tilted grayscale texture!! https://stackoverflow.com/questions/15983607/opengl-texture-tilted
    //Generate an array of textures.  We only want one texture (one element array), so trick
    //it by treating "texture" as array of length one.
    glGenTextures(1,&texture);
    //Select (bind) the texture we just generated as the current 2D texture OpenGL is using/modifying.
    //All subsequent changes to OpenGL's texturing state for 2D textures will affect this texture.
    glBindTexture(GL_TEXTURE_2D,texture);
    //Specify the texture's data.  This function is a bit tricky, and it's hard to find helpful documentation.  A summary:
    //   GL_TEXTURE_2D:    The currently bound 2D texture (i.e. the one we just made)
    //               0:    The mipmap level.  0, since we want to update the base level mipmap image (i.e., the image itself,
    //                         not cached smaller copies)
    //         GL_RGBA:    The internal format of the texture.  This is how OpenGL will store the texture internally (kinda)--
    //                         it's essentially the texture's type.
    //         surf->w:    The width of the texture
    //         surf->h:    The height of the texture
    //               0:    The border.  Don't worry about this if you're just starting.
    //        data_fmt:    The format that the *data* is in--NOT the texture!  Our test image doesn't have an alpha channel,
    //                         so this must be RGB.
    //GL_UNSIGNED_BYTE:    The type the data is in.  In SDL, the data is stored as an array of bytes, with each channel
    //                         getting one byte.  This is fairly typical--it means that the image can store, for each channel,
    //                         any value that fits in one byte (so 0 through 255).  These values are to be interpreted as
    //                         *unsigned* values (since 0x00 should be dark and 0xFF should be bright).
    // surface->pixels:    The actual data.  As above, SDL's array of bytes.
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surf->w,surf->h, 0, data_fmt, GL_UNSIGNED_BYTE, surf->pixels);
    //Set the minification and magnification filters.  In this case, when the texture is minified (i.e., the texture's pixels (texels) are
    //*smaller* than the screen pixels you're seeing them on, linearly filter them (i.e. blend them together).  This blends four texels for
    //each sample--which is not very much.  Mipmapping can give better results.  Find a texturing tutorial that discusses these issues
    //further.  Conversely, when the texture is magnified (i.e., the texture's texels are *larger* than the screen pixels you're seeing
    //them on), linearly filter them.  Qualitatively, this causes "blown up" (overmagnified) textures to look blurry instead of blocky.
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    //Unload SDL's copy of the data; we don't need it anymore because OpenGL now stores it in the texture.
    SDL_FreeSurface(surf);
}

glImage::~glImage() {
    //Deallocate texture.  A lot of people forget to do this.
    glDeleteTextures(1,&texture);
}

vec::vec() {
    x, y, z, w = 0;
}
vec::vec(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
aabb::aabb(point center, point halfwidths): c(center), hw(halfwidths) {};

/// @brief Default Ent Behaviour (collision)
void deb_collision(ent* a, ent* b) {
    assert(a);
    assert(b);
    if(a->collider.overlap(&b->collider))
    {
        /// @todo prevent ghosting ( add collision behaviour )
    }
}
bool aabb::overlap(aabb* b) {
    assert(b);
    if ( std::abs(this->c.x - b->c.x) > (this->hw.x + b->hw.x)
    or std::abs(this->c.y - b->c.y) > (this->hw.y + b->hw.y)
    or std::abs(this->c.z - b->c.z) > (this->hw.z + b->hw.z)
    ) return false;
    return true;
}
log::log(uint16_t log_level) : level(log_level), sink(&log::console_) { assert(sink); }

void log::console_(std::string str, uint16_t limit) {
    if(limit<=level) printf("%s", (limit==3?"[WARNING] ":(limit==2?"[INFO] ":(limit==1?"[ERROR] ":""))+str+"\n").c_str());
}
void log::err(std::string str) {
    std::invoke(sink, *this, str, 1);
}
void log::info(std::string str) {
    std::invoke(sink, *this, str, 2);
}
void log::warn(std::string str) {
    std::invoke(sink, *this, str, 3);
}
void ent::w_append_(world* w) {
    assert(w);
    this->w=w;
    e_uuid = uuid::new_v4();
    w->ents.insert({e_uuid, std::unique_ptr<ent>(this)});
}
void ent::w_erase_() {
    assert(w);
    assert(&w->ents);
    assert(&e_uuid);
    // Обязательно вызывать release() у unique_ptr при удалении
    w->ents.at(e_uuid).release();
    w->ents.erase(e_uuid);
}
ent::ent(world* w, void(*on_create)(ent*)) {
        invoke_(on_create); 
        w_append_(w); 
}
ent::~ent() {
        invoke_(on_delete); 
        w_erase_(); 
}
void ent::update() { invoke_(on_update); }
void ent::draw() { invoke_(on_draw); }
/// @deprecated
void deb_draw_collider(ImGuiIO& io, aabb& collider) {
    imPushMatrix();
    glTranslatef(collider.c.x, collider.c.y, collider.c.z);
    glBegin(GL_POLYGON);
        glColor4ub(255, 0, 0, 255);
        glVertex2f(-collider.hw.x, -collider.hw.y);
        glVertex2f(collider.hw.x, -collider.hw.y);
        glVertex2f(collider.hw.x, collider.hw.y);
        glVertex2f(-collider.hw.x, collider.hw.y);
    glEnd();
    glPopMatrix();
}
// body::body(world* w) : game(w), position(0.0, 0.0, 0.0) { append_to_world_(); }
// body::body(world* w, vec3f position) : game(w), position(position) { append_to_world_(); }
// body::body(world* w, float x, float y, float z) : game(w), position(x, y, z) { append_to_world_(); }
// AABBComponent<body>::AABBComponent(aabb collider) : collider(collider) {}