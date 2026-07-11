#pragma once

#include <string>
#include <unordered_map>

namespace bui {

class Shader {
public:
    Shader() = default;
    ~Shader();

    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

    bool Load(const std::string& vertexSrc, const std::string& fragmentSrc);
    bool LoadFromFiles(const std::string& vertexPath, const std::string& fragmentPath);

    void Use() const;
    void SetInt(const std::string& name, int value);
    void SetFloat(const std::string& name, float value);
    void SetVec2(const std::string& name, float x, float y);
    void SetVec3(const std::string& name, float x, float y, float z);
    void SetVec4(const std::string& name, float x, float y, float z, float w);
    void SetMat4(const std::string& name, const float* mat);

    unsigned int Program() const { return program_; }

private:
    bool Compile(unsigned int& shader, const std::string& src, unsigned int type);
    int GetUniformLocation(const std::string& name);

    unsigned int program_ = 0;
    std::unordered_map<std::string, int> uniformCache_;
};

} // namespace bui
