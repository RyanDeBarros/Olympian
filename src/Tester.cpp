#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stb/stb_image.h>

#include "rendering/core/Batches.h"
#include "rendering/core/Textures.h"

struct SpriteCPUData
{
	// vbo 0
	mutable GLubyte vertex_tex_slots[4] = { 0, 0, 0, 0 };
	// vbo 1
	GLubyte vertex_tex_coord_slots[4] = { 0, 0, 0, 0 };
	// vbo 2
	glm::mat3 vertex_transforms[4] = { glm::mat3(1.0f), glm::mat3(1.0f), glm::mat3(1.0f), glm::mat3(1.0f) };

	std::shared_ptr<oly::rendering::Texture> texture; // TODO eventually, std::vector<std::shared_ptr<oly::rendering::Texture>>
	glm::vec2 texture_dimensions[1]; // TODO use vector
	oly::rendering::GLBuffer tex_dimensions_ubo;
	std::array<glm::vec2, 4> texture_coords[1]; // TODO use vector<array<glm::vec2, 4>>
	oly::rendering::GLBuffer tex_coords_ssbo;

	void load(float w, float h)
	{
		texture_dimensions[0].x = w;
		texture_dimensions[0].y = h;
		glBindBuffer(GL_UNIFORM_BUFFER, tex_dimensions_ubo);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(texture_dimensions), texture_dimensions, GL_DYNAMIC_DRAW);
		
		texture_coords[0][0].x = 0.0f;
		texture_coords[0][0].y = 0.0f;
		texture_coords[0][1].x = 1.0f;
		texture_coords[0][1].y = 0.0f;
		texture_coords[0][2].x = 1.0f;
		texture_coords[0][2].y = 1.0f;
		texture_coords[0][3].x = 0.0f;
		texture_coords[0][3].y = 1.0f;
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, tex_coords_ssbo);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(texture_coords), texture_coords, GL_STATIC_DRAW);
	}

	const unsigned short indices[6] = {
		0, 1, 2,
		2, 3, 0
	};

	struct
	{
		GLenum mode = GL_TRIANGLES;
		GLuint indices = 6;
		GLenum type = GL_UNSIGNED_SHORT;
		GLuint offset = 0;
	} draw_spec;
};

typedef oly::rendering::Batch<SpriteCPUData> SpriteBatch;

template<>
void oly::rendering::draw(const SpriteBatch& batch)
{
	const auto& data = batch.cpu_data;

	glActiveTexture(GL_TEXTURE0 + 0);
	glBindTexture(GL_TEXTURE_2D, *data.texture);
	//glBindTextureUnit(GL_TEXTURE0 + 0, vdata.texture); // TODO only available in 4.5+, so use macros. also explorer bindless textures instead of sampler2d[]

	for (size_t i = 0; i < 4; ++i)
		data.vertex_tex_slots[i] = 0;
	glBindBuffer(GL_ARRAY_BUFFER, batch.get_vbo(0));
	glBufferData(GL_ARRAY_BUFFER, sizeof(data.vertex_tex_slots), &data.vertex_tex_slots, GL_DYNAMIC_DRAW);

	// TODO create ShaderData class that references Shader and tracks uniform location and bindings. That way, can use glUniformBlockBinding on shader startup and not call glBindBufferBase every frame.
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, data.tex_dimensions_ubo);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, data.tex_coords_ssbo);
	glDrawElements(data.draw_spec.mode, data.draw_spec.indices, data.draw_spec.type, (void*)data.draw_spec.offset);
}

template<>
void oly::rendering::attrib_layout(const SpriteBatch& batch)
{
	glBindBuffer(GL_ARRAY_BUFFER, batch.get_vbo(0));
	glVertexAttribPointer(0, 1, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(GLubyte), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, batch.get_vbo(1));
	glVertexAttribPointer(1, 1, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(GLubyte), (void*)0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, batch.get_vbo(2));
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(glm::mat3), (void*)0);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(glm::mat3), (void*)sizeof(glm::vec3));
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(glm::mat3), (void*)(2 * sizeof(glm::vec3)));
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);
	glEnableVertexAttribArray(4);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

static void init_buffers(const SpriteBatch& batch)
{
	const auto& data = batch.cpu_data;

	glBindBuffer(GL_ARRAY_BUFFER, batch.get_vbo(0));
	glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(GLubyte), &data.vertex_tex_slots, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, batch.get_vbo(1));
	glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(GLubyte), &data.vertex_tex_coord_slots, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, batch.get_vbo(2));
	glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(glm::mat3), glm::value_ptr(data.vertex_transforms[0]), GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, batch.get_ebo());
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned short), data.indices, GL_STATIC_DRAW);
}

int main()
{
	if (glfwInit() != GLFW_TRUE)
		return 1;

	stbi_set_flip_vertically_on_load(true);

	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
	GLFWwindow* window = glfwCreateWindow(1440, 1080, "Olympian Engine", nullptr, nullptr);
	if (!window)
		return 2;

	glfwMakeContextCurrent(window);

	if (glewInit() != GLEW_OK)
		return 3;
	glClearColor(0.2f, 0.5f, 0.8f, 1.0f);
	glfwSwapInterval(1);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	int w, h, chpp;
	unsigned char* image = stbi_load("../../../res/textures/einstein.png", &w, &h, &chpp, 4);
	auto texture = std::make_shared<oly::rendering::Texture>(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, *texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	GLenum internal_format =
		chpp == 1 ? GL_R8
		: chpp == 2 ? GL_RG8
		: chpp == 3 ? GL_RGB8
		: GL_RGBA8;
	GLenum format =
		chpp == 1 ? GL_RED
		: chpp == 2 ? GL_RG
		: chpp == 3 ? GL_RGB
		: GL_RGBA;
	GLint alignment =
		chpp == 1 ? 1
		: chpp == 2 ? 2
		: chpp == 3 ? 1
		: 4;
	glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);
	glTexImage2D(GL_TEXTURE_2D, 0, internal_format, w, h, 0, format, GL_UNSIGNED_BYTE, image);
	stbi_image_free(image);

	SpriteBatch batch;
	batch.shader = oly::rendering::load_shader("../../../src/shaders/sprite_2d.vert", "../../../src/shaders/sprite_2d.frag");
	batch.gen_vao_descriptor(3, true);
	batch.cpu_data.load(w, h);
	batch.cpu_data.texture = texture;
	init_buffers(batch);

	glm::mat3 proj = glm::ortho<float>(-720, 720, -540, 540);
	GLuint proj_location = glGetUniformLocation(batch.get_shader(), "uProjection");
	glUseProgram(batch.get_shader());
	glUniformMatrix3fv(proj_location, 1, GL_FALSE, glm::value_ptr(proj));

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		glUseProgram(batch.get_shader());
		glBindVertexArray(batch.get_vao());
		oly::rendering::draw(batch);

		glfwSwapBuffers(window);
		glClear(GL_COLOR_BUFFER_BIT);
	}

	glfwTerminate();
	return 0;
}
