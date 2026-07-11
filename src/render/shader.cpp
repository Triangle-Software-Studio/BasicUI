#include "basicui/render/shader.h"
#include <GL/glew.h>
#include <fstream>
#include <sstream>
#include <iostream>

namespace bui {

Shader::~Shader() {
    if (program_) {
        glDeleteProgram(program_);
    }
}

bool Shader::Load(const std::string& vertexSrc, const std::string& fragmentSrc) {
    if (program_) {
        glDeleteProgram(program_);
        program_ = 0;
    }
    uniformCache_.clear();

    unsigned int vs = 0, fs = 0;
    if (!Compile(vs, vertexSrc, GL_VERTEX_SHADER)) return false;
    if (!Compile(fs, fragmentSrc, GL_FRAGMENT_SHADER)) return false;

    program_ = glCreateProgram();
    glAttachShader(program_, vs);
    glAttachShader(program_, fs);
    glLinkProgram(program_);

    int success = 0;
    glGetProgramiv(program_, GL_LINK_STATUS, &success);
    if (!success) {
        char log[512];
        glGetProgramInfoLog(program_, 512, nullptr, log);
        std::cerr << "Shader link error: " << log << "\n";
        glDeleteProgram(program_);
        program_ = 0;
        glDeleteShader(vs);
        glDeleteShader(fs);
        return false;
    }

    glDeleteShader(vs);
    glDeleteShader(fs);
    return true;
}

bool Shader::LoadFromFiles(const std::string& vertexPath, const std::string& fragmentPath) {
    std::ifstream vf(vertexPath), ff(fragmentPath);
    if (!vf || !ff) return false;
    std::stringstream vss, fss;
    vss << vf.rdbuf();
    fss << ff.rdbuf();
    return Load(vss.str(), fss.str());
}

void Shader::Use() const {
    if (program_) glUseProgram(program_);
}

void Shader::SetInt(const std::string& name, int value) {
    glUniform1i(GetUniformLocation(name), value);
}

void Shader::SetFloat(const std::string& name, float value) {
    glUniform1f(GetUniformLocation(name), value);
}

void Shader::SetVec2(const std::string& name, float x, float y) {
    glUniform2f(GetUniformLocation(name), x, y);
}

void Shader::SetVec3(const std::string& name, float x, float y, float z) {
    glUniform3f(GetUniformLocation(name), x, y, z);
}

void Shader::SetVec4(const std::string& name, float x, float y, float z, float w) {
    glUniform4f(GetUniformLocation(name), x, y, z, w);
}

void Shader::SetMat4(const std::string& name, const float* mat) {
    glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, mat);
}

bool Shader::Compile(unsigned int& shader, const std::string& src, unsigned int type) {
    shader = glCreateShader(type);
    const char* cstr = src.c_str();
    glShaderSource(shader, 1, &cstr, nullptr);
    glCompileShader(shader);
    int success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[512];
        glGetShaderInfoLog(shader, 512, nullptr, log);
        std::cerr << "Shader compile error: " << log << "\n";
        glDeleteShader(shader);
        shader = 0;
        return false;
    }
    return true;
}

int Shader::GetUniformLocation(const std::string& name) {
    auto it = uniformCache_.find(name);
    if (it != uniformCache_.end()) return it->second;
    int loc = glGetUniformLocation(program_, name.c_str());
    uniformCache_[name] = loc;
    return loc;
}

} // namespace bui
