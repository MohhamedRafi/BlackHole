```text
┌────────────────────────────────────────────────────────────────────┐
│                            main.cpp                                │
│--------------------------------------------------------------------│
│  Engine E;                                                         │
│  E.go(Boot) → InitGL → Loading → Running → ShuttingDown            │
│  while (E.running) { tick_fixed(); update(); render(); }           │
└────────────────────────────────────────────────────────────────────┘
                                   │
                                   ▼
                 ┌────────────────────────────────┐
                 │           Engine               │
                 │--------------------------------│
                 │ Boot: log start                │
                 │ InitGL: glfwInit, glad,        │
                 │   create window, viewport,     │
                 │   setup_input_callbacks()      │
                 │ Loading: shaders.load_all();   │
                 │   renderer.init_triangle();    │
                 │   camera.updateVectors();      │
                 │ Running: main loop active      │
                 │ ShuttingDown: cleanup, exit    │
                 └────────────────────────────────┘
                                   │
                                   ▼
      ┌─────────────────────────────────────────────────────┐
      │                 Engine Main Loop                    │
      │-----------------------------------------------------│
      │ tick_fixed(): angle += ω·dt                         │
      │ update(): process WASD + mouse → camera.move/look   │
      │ render(): renderer.draw(angle, camera.getViewProj())│
      │ swap buffers, poll events                           │
      └─────────────────────────────────────────────────────┘
                                   │
                                   ▼
      ┌─────────────────────────────────────────────────────┐
      │                 Input Callbacks                     │
      │-----------------------------------------------------│
      │ CursorPos(x,y): camera.processMouse(dx,dy)          │
      │ FramebufferSize(w,h): glViewport, camera.aspect=h/w │
      │ ESC → toggle captureMouse                           │
      └─────────────────────────────────────────────────────┘
                                   │
                                   ▼
      ┌─────────────────────────────────────────────────────┐
      │                     Camera (GLM)                    │
      │-----------------------------------------------------│
      │ yaw/pitch → updateVectors() → front/right/up        │
      │ getView() = lookAt(pos, pos+front, up)              │
      │ getProj() = perspective(fov, aspect, zn, zf)        │
      │ getViewProj() = P * V                               │
      └─────────────────────────────────────────────────────┘
                                   │
                                   ▼
      ┌─────────────────────────────────────────────────────┐
      │                   Renderer                          │
      │-----------------------------------------------------│
      │ init_triangle(lib):                                 │
      │   prog = lib.get_flat_color().id                    │
      │   set up VAO/VBO with aPos(0), aCol(1)              │
      │ draw(angle, VP):                                    │
      │   M   = rotateZ(angle)                              │
      │   MVP = VP * M                                      │
      │   glUseProgram(prog);                               │
      │   glUniformMatrix4fv(uTransform, MVP);              │
      │   glBindVertexArray(vao);                           │
      │   glDrawArrays(GL_TRIANGLES, 0, 3);                 │
      └─────────────────────────────────────────────────────┘
                                   │
                                   ▼
      ┌─────────────────────────────────────────────────────┐
      │                 ShaderLibrary / Shader              │
      │-----------------------------------------------------│
      │ load_all(): compile + link flat.vert/frag           │
      │ Shader.compile(): glCreateShader + glCompileShader  │
      │ Shader.link(): glAttach + glLinkProgram             │
      │ uses AssetLoader to read GLSL from ASSET_DIR        │
      │ bind()/unbind() → glUseProgram(id/0)                │
      └─────────────────────────────────────────────────────┘
                                   │
                                   ▼
      ┌─────────────────────────────────────────────────────┐
      │                    AssetLoader                      │
      │-----------------------------------------------------│
      │ resolve("rel") = ASSET_DIR/rel                      │
      │ read_text() → std::string(shader source)            │
      └─────────────────────────────────────────────────────┘
                                   │
                                   ▼
      ┌─────────────────────────────────────────────────────┐
      │                     OpenGL Driver                   │
      │-----------------------------------------------------│
      │ Executes linked GPU programs                        │
      │ Renders triangles from VAO using active shaders     │
      └─────────────────────────────────────────────────────┘
