#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include <psl/fstream.h>

#include <pine/vecmath.h>
#include <pine/fileio.h>
#include <pine/log.h>

using namespace pine;

struct GLWindow {
  GLWindow(vec2i size, str title) {
    glfwSetErrorCallback(+[](int, const char* description) { FATAL("GLFW Error: ", description); });
    if (!glfwInit()) FATAL("Unable to initialize GLFW");

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_SAMPLES, 4);

    window = glfwCreateWindow(size.x, size.y, title.c_str(), nullptr, nullptr);
    if (window == nullptr) FATAL("GLFW: unable to create window");
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    if (gladLoadGL() == 0) FATAL("GLAD: unable to load GL function address");

    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(
        [](GLenum source, GLenum, GLuint, GLenum severity, GLsizei, const GLchar* message,
           const void*) {
          if (source != GL_DEBUG_SOURCE_SHADER_COMPILER)
            if (severity == GL_DEBUG_SEVERITY_MEDIUM || severity == GL_DEBUG_SEVERITY_HIGH)
              FATAL("GL: ", message);
        },
        0);

    glEnable(GL_MULTISAMPLE);
    glClearColor(0, 0, 0, 0);
  }
  ~GLWindow() { glfwDestroyWindow(window); }
  GLWindow(GLWindow&&) = delete;

  void update() const {
    glfwPollEvents();

    glfwSwapBuffers(window);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  }

  bool should_close() const { return glfwWindowShouldClose(window); }

  bool is_key_pressed(int key) const { return glfwGetKey(window, key) == GLFW_PRESS; }

 private:
  GLFWwindow* window;
};

struct GLShader {
  enum Type { Vertex, Fragment };
  GLShader(Type type, str content) {
    switch (type) {
      case Vertex:
        shader = glCreateShader(GL_VERTEX_SHADER);
        break;
      case Fragment:
        shader = glCreateShader(GL_FRAGMENT_SHADER);
        break;
      default:
        PINE_UNREACHABLE;
    }
    auto length = int(content.size());
    const char* content_array[]{content.c_str()};
    glShaderSource(shader, 1, content_array, &length);
    glCompileShader(shader);

    char info[1024];
    GLsizei info_length;
    glGetShaderInfoLog(shader, 1024, &info_length, info);
    if (info_length) FATAL("GL shader:\n", info);
  }
  ~GLShader() { glDeleteShader(shader); }
  GLShader(GLShader&&) = delete;

  GLuint shader;
};

struct GLProgram {
  GLProgram(const GLShader& vshader, const GLShader& fshader) {
    program = glCreateProgram();
    glAttachShader(program, vshader.shader);
    glAttachShader(program, fshader.shader);
    glLinkProgram(program);

    char info[1024];
    GLsizei info_length;
    glGetProgramInfoLog(program, 1024, &info_length, info);
    if (info_length) FATAL("GL program: ", info);

    glDetachShader(program, vshader.shader);
    glDetachShader(program, fshader.shader);

    glUseProgram(program);
  }
  ~GLProgram() { glDeleteProgram(program); }
  GLProgram(GLProgram&&) = delete;

  void use() const { glUseProgram(program); }

  void set_uniform(const char* name, float value) const {
    auto loc = glGetUniformLocation(program, name);
    if (loc == -1) FATAL("GLProgram: uniform `", name, "` not found");
    glUniform1f(loc, value);
  }
  void set_uniform(const char* name, vec3 value) const {
    auto loc = glGetUniformLocation(program, name);
    if (loc == -1) FATAL("GLProgram: uniform `", name, "` not found");
    glUniform3f(loc, value[0], value[1], value[2]);
  }
  void set_uniform(const char* name, mat4 value) const {
    auto loc = glGetUniformLocation(program, name);
    if (loc == -1) FATAL("GLProgram: uniform `", name, "` not found");
    glUniformMatrix4fv(loc, 1, false, &value[0][0]);
  }

  GLuint program;
};

struct VBO {
  VBO(size_t size, void* data) {
    glCreateBuffers(1, &vbo);
    bind();
    glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
  }
  ~VBO() { glDeleteBuffers(1, &vbo); }
  VBO(VBO&& rhs) : vbo(psl::exchange(rhs.vbo, 0)) {}

  void bind() const { glBindBuffer(GL_ARRAY_BUFFER, vbo); }

  GLuint vbo = 0;
};

struct VAO {
  VAO(const VBO& vbo) {
    vbo.bind();
    glCreateVertexArrays(1, &vao);
    bind();
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
  }
  ~VAO() { glDeleteVertexArrays(1, &vao); }
  VAO(VAO&& rhs) : vao(psl::exchange(rhs.vao, 0)) {}

  void bind() const { glBindVertexArray(vao); }

  GLuint vao = 0;
};

struct Mesh {
  Mesh(vec2 v0, auto... vs) : vertices(psl::vector_of<vec3>(vec3(v0), vec3(vs)...)) {}
  Mesh(vec3 v0, auto... vs) : vertices(psl::vector_of<vec3>(v0, vs...)) {}
  Mesh(psl::vector<vec2> vs) : vertices(vs.size()) {
    for (size_t i = 0; i < vs.size(); i++) vertices[i] = vec3(vs[i]);
  }
  Mesh(psl::vector<vec3> vs) : vertices(MOVE(vs)) {}

  Mesh translate(vec3 value) {
    auto copy = *this;
    for (auto& v : copy.vertices) v += value;
    return copy;
  }
  Mesh scale(vec3 value) {
    auto copy = *this;
    for (auto& v : vertices) v *= value;
    return copy;
  }
  Mesh rotate(float x, float y, float z) {
    auto copy = *this;
    auto m = pine::rotate(vec3(x, y, z));
    for (auto& v : vertices) v = m * v;
    return copy;
  }

  psl::vector<vec3> vertices;
};

struct Model {
  struct TriangleFan {
    TriangleFan(Mesh mesh_)
        : mesh(MOVE(mesh_)),
          vbo(mesh.vertices.size() * sizeof(vec3), &mesh.vertices[0]),
          vao(vbo) {}

    void draw() const {
      vao.bind();
      glDrawArrays(GL_TRIANGLE_FAN, 0, mesh.vertices.size());
    }

    Mesh mesh;
    VBO vbo;
    VAO vao;
  };

  Model(vec3 position, vec3 color, auto... meshes)
      : position(position), color(color), fans(psl::vector_of(TriangleFan(MOVE(meshes))...)) {}
  void draw(const GLProgram& program) const {
    program.set_uniform("translate", position);
    program.set_uniform("color", color);
    for (auto& fan : fans) {
      fan.draw();
    }
  }

 private:
  vec3 position = vec3(0.0f, 0.0f, 1.0f);
  vec3 color = vec3(1.0f, 0.0f, 1.0f);
  psl::vector<TriangleFan> fans;
};

template <typename... Ts>
auto create_model(vec3 position, vec3 color, Ts... meshes) {
  return Model(position, color, MOVE(meshes)...);
}

Mesh gen_mesh(int segments, auto vertex_generator) {
  auto vertices = psl::vector<vec2>(segments);
  for (int i = 0; i < segments; i++) vertices[i] = vertex_generator(i);
  return Mesh(vertices);
}
Mesh create_circle(vec2 center, float radius, int segments) {
  return gen_mesh(segments, [&](int i) {
    float rad = (Pi2 * i) / segments;
    return center + radius * vec2(psl::cos(rad), psl::sin(rad));
  });
}

struct Scene {
  void add(Model model) { models.push_back(MOVE(model)); }
  void draw(const GLProgram& program) const {
    for (const auto& model : models) model.draw(program);
  }

 private:
  psl::vector<Model> models;
};

vec3 pos = vec3(0, 0, -1);
vec3 dir = vec3(0, 0, 1);

int main() {
  auto window = GLWindow({600, 600}, "Hello");
  auto scene = Scene();

  auto trapezoid =
      Mesh(vec2(-0.1f, 0.5f), vec2(-0.07f, -0.5f), vec2(0.07f, -0.5f), vec2(0.1f, 0.5f));
  auto circle = create_circle(vec2(0.0f, -0.7f), 0.1f, 32);

  scene.add(create_model(vec3(0, 0, 0), vec3(0.8f, 0.4f, 0.1f), trapezoid, circle));
  scene.add(create_model(vec3(1, 0, 1), vec3(0.8f, 0.4f, 0.1f), trapezoid, circle));
  scene.add(create_model(vec3(2, 0, 2), vec3(0.8f, 0.4f, 0.1f), trapezoid, circle));
  scene.add(create_model(vec3(3, 0, 3), vec3(0.8f, 0.4f, 0.1f), trapezoid, circle));

  auto vshader = GLShader(GLShader::Vertex, read_str("shaders/basic.vert"));
  auto fshader = GLShader(GLShader::Fragment, read_str("shaders/basic.frag"));
  auto program = GLProgram(vshader, fshader);

  while (!window.should_close()) {
    if (window.is_key_pressed(GLFW_KEY_UP)) pos.y += 0.01f;
    if (window.is_key_pressed(GLFW_KEY_DOWN)) pos.y -= 0.01f;

    if (window.is_key_pressed(GLFW_KEY_A)) pos.x -= 0.01f;
    if (window.is_key_pressed(GLFW_KEY_D)) pos.x += 0.01f;

    if (window.is_key_pressed(GLFW_KEY_W)) pos.z += 0.01f;
    if (window.is_key_pressed(GLFW_KEY_S)) pos.z -= 0.01f;

    program.set_uniform("view", inverse(look_at(pos, pos + dir)));
    scene.draw(program);
    window.update();
  }

  return 0;
}