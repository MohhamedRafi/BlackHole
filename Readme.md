┌──────────────────────────────────────────────┐
│                  main.cpp                    │
│----------------------------------------------│
│ int main() {                                 │
│   Engine engine;                             │
│   engine.run();   ← starts the state machine │
│ }                                            │
└──────────────────────────────────────────────┘
                      │
                      ▼
┌──────────────────────────────────────────────┐
│                  Engine                      │
│----------------------------------------------│
│ enum class EngineState { Boot, InitGL, ... } │
│ ShaderLibrary shaders;                       │
│ Renderer renderer;                           │
│----------------------------------------------│
│ on_enter(Boot)    → "Boot" message           │
│ on_enter(InitGL)  → initialize GLFW/GLAD     │
│ on_enter(Loading) → shaders.load_all();      │
│                    renderer.init_triangle(); │
│ on_enter(Running) → renderer.draw() loop     │
└──────────────────────────────────────────────┘
                      │
                      ▼
┌──────────────────────────────────────────────┐
│              ShaderLibrary                   │
│----------------------------------------------│
│ Shader flat_color;                           │
│----------------------------------------------│
│ bool load_all() {                            │
│   flat_color.load("flat.vert","flat.frag");  │
│ }                                            │
│ const Shader& get_flat_color() const;        │
└──────────────────────────────────────────────┘
                      │
                      ▼
┌──────────────────────────────────────────────┐
│                  Shader                      │
│----------------------------------------------│
│ GLuint id;                                   │
│----------------------------------------------│
│ load(v_path, f_path):                        │
│   1. Read GLSL sources                       │
│   2. glCreateShader(GL_VERTEX_SHADER)        │
│   3. glShaderSource + glCompileShader        │
│   4. Repeat for fragment shader              │
│   5. glCreateProgram + glAttachShader        │
│   6. glLinkProgram → id                      │
│   7. Delete intermediate shaders             │
│----------------------------------------------│
│ bind()   → glUseProgram(id)                  │
│ unbind() → glUseProgram(0)                   │
│ destroy() → glDeleteProgram(id)              │
└──────────────────────────────────────────────┘
                      │
                      ▼
┌──────────────────────────────────────────────┐
│                 Renderer                     │
│----------------------------------------------│
│ GLuint vao, vbo, prog;                       │
│----------------------------------------------│
│ init_triangle(lib):                          │
│   prog = lib.get_flat_color().id             │
│   glGenVertexArrays, glBind, glBufferData    │
│   glVertexAttribPointer for pos/color        │
│ draw():                                      │
│   glUseProgram(prog)                         │
│   glBindVertexArray(vao)                     │
│   glDrawArrays(GL_TRIANGLES, 0, 3)           │
└──────────────────────────────────────────────┘
                      │
                      ▼
┌──────────────────────────────────────────────┐
│                OpenGL Driver                 │
│----------------------------------------------│
│ Executes compiled shader programs on GPU.    │
│ Renders vertices stored in GPU buffers.      │
└──────────────────────────────────────────────┘
