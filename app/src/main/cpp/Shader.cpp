#include "Shader.h"

#include "AndroidOut.h"
#include "Model.h"
#include "Utility.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
Shader *Shader::loadShader(
        const std::string &vertexSource,
        const std::string &fragmentSource,
        const std::string &positionAttributeName,
        const std::string &modelAttributeName,
        const std::string &projectionMatrixUniformName) {
    Shader *shader = nullptr;

    GLuint vertexShader = loadShader(GL_VERTEX_SHADER, vertexSource);
    if (!vertexShader) {
        return nullptr;
    }

    GLuint fragmentShader = loadShader(GL_FRAGMENT_SHADER, fragmentSource);
    if (!fragmentShader) {
        glDeleteShader(vertexShader);
        return nullptr;
    }

    GLuint program = glCreateProgram();
    if (program) {
        glAttachShader(program, vertexShader);
        glAttachShader(program, fragmentShader);

        glLinkProgram(program);
        GLint linkStatus = GL_FALSE;
        glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
        if (linkStatus != GL_TRUE) {
            GLint logLength = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);

            // If we fail to link the shader program, log the result for debugging
            if (logLength) {
                GLchar *log = new GLchar[logLength];
                glGetProgramInfoLog(program, logLength, nullptr, log);
                aout << "Failed to link program with:\n" << log << std::endl;
                delete[] log;
            }

            glDeleteProgram(program);
        } else {
            // Get the attribute and uniform locations by name. You may also choose to hardcode
            // indices with layout= in your shader, but it is not done in this sample
            GLint positionAttribute = glGetAttribLocation(program, positionAttributeName.c_str());
            GLint projectionMatrixUniform = glGetUniformLocation(
                    program,
                    projectionMatrixUniformName.c_str());

            GLint modelMatrixUniform = glGetUniformLocation(
                    program,
                    modelAttributeName.c_str());

            // Only create a new shader if all the attributes are found.
            if (positionAttribute != -1 &&
                modelMatrixUniform != -1
                && projectionMatrixUniform != -1) {

                shader = new Shader(
                        program,
                        positionAttribute,
                        modelMatrixUniform,
                        projectionMatrixUniform);
            } else {
                aout << "FAILEEEEEEEEEEEEEEEEEEEED ============================" << std::endl;
                glDeleteProgram(program);
            }
        }
    }

    // The shaders are no longer needed once the program is linked. Release their memory.
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shader;
}

GLuint Shader::loadShader(GLenum shaderType, const std::string &shaderSource) {
    Utility::assertGlError();
    GLuint shader = glCreateShader(shaderType);
    if (shader) {
        auto *shaderRawString = (GLchar *) shaderSource.c_str();
        GLint shaderLength = shaderSource.length();
        glShaderSource(shader, 1, &shaderRawString, &shaderLength);
        glCompileShader(shader);

        GLint shaderCompiled = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &shaderCompiled);

        // If the shader doesn't compile, log the result to the terminal for debugging
        if (!shaderCompiled) {
            GLint infoLength = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLength);

            if (infoLength) {
                auto *infoLog = new GLchar[infoLength];
                glGetShaderInfoLog(shader, infoLength, nullptr, infoLog);
                aout << "Failed to compile with:\n" << infoLog << std::endl;
                delete[] infoLog;
            }

            glDeleteShader(shader);
            shader = 0;
        }
    }
    return shader;
}

void Shader::activate() const {
    glUseProgram(program_);
}

void Shader::deactivate() const {
    glUseProgram(0);
}

void Shader::drawModel(const Model &model) const {
    // The position attribute is 3 floats
    glVertexAttribPointer(
            position_, // attrib
            3, // elements
            GL_FLOAT, // of type float
            GL_FALSE, // don't normalize
            sizeof(Vertex), // stride is Vertex bytes
            model.getVertexData() // pull from the start of the vertex data
    );
    glEnableVertexAttribArray(position_);


    // Draw as indexed triangles
    glDrawElements(GL_TRIANGLES, model.getIndexCount(), GL_UNSIGNED_SHORT, model.getIndexData());
}

void Shader::setProjectionMatrix(float h, float w) const {

    glm::vec2 size = glm::vec2(50.0f, 50.0f);

    glm::mat4 ml = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
    ml = glm::translate(ml, glm::vec3( 0 , h - 200, 0.0f));


    ml = glm::scale(ml, glm::vec3(glm::vec2(200.0f, 200.0f), 1.0f));

    glm::mat4 projection(1.0f);
    projection = glm::ortho(0.0f, (float)w, (float)h, 0.0f, -1.0f, 0.0f);

    glUniformMatrix4fv(projectionMatrix_, 1, false, glm::value_ptr(projection));

    glUniformMatrix4fv(model_, 1, false, glm::value_ptr(ml));
}