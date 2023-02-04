/*!
 * @file
 * @brief This file contains implementation of gpu
 *
 * @author Tomáš Milet, imilet@fit.vutbr.cz
 */

#include <student/gpu.hpp>
#include <iostream>

typedef struct Triangle{
	OutVertex points[3];
} Primitive;

typedef std::vector<Primitive> Clipped;

uint32_t computeVertexID(VertexArray const& vao, uint32_t shaderInvocation) {
	if (!vao.indexBuffer) 
		return shaderInvocation;
	
	if (vao.indexType == IndexType::UINT32) {
		uint32_t* ind = (uint32_t*)vao.indexBuffer;
		return ind[shaderInvocation];
	}
	if (vao.indexType == IndexType::UINT16) {
		uint16_t* ind = (uint16_t*)vao.indexBuffer;
		return ind[shaderInvocation];
	}
	if (vao.indexType == IndexType::UINT8) {
		uint8_t* ind = (uint8_t*)vao.indexBuffer;
		return ind[shaderInvocation];
	}
}

void readAttributes(InVertex &in, VertexArray &vao) {
	for (int i = 0; i < maxAttributes; i++) {
		const auto& attribute = vao.vertexAttrib[i];
		auto index = attribute.offset + in.gl_VertexID * attribute.stride;
		const auto& buffer = (uint8_t*)attribute.bufferData + index;
		switch (attribute.type) {
			case AttributeType::EMPTY:
				break;
			case AttributeType::FLOAT:
				in.attributes[i].v1 = *(float*)buffer;
				break;
			case AttributeType::VEC2:
				in.attributes[i].v2 = *(glm::vec2*)buffer;
				break;
			case AttributeType::VEC3:
				in.attributes[i].v3 = *(glm::vec3*)buffer;
				break;
			case AttributeType::VEC4:
				in.attributes[i].v4 = *(glm::vec4*)buffer;
				break;
		}
	}
}

void interpolateAttributes(Primitive &primitive, Program &prg, InFragment &inFragment, float &deltaA, float &deltaB, float &deltaC) {
		float s = deltaA / primitive.points[0].gl_Position.w + deltaB / primitive.points[1].gl_Position.w + deltaC / primitive.points[2].gl_Position.w;
		float lambdaA = deltaA / (primitive.points[0].gl_Position.w * s);
		float lambdaB = deltaB / (primitive.points[1].gl_Position.w * s);
		float lambdaC = deltaC / (primitive.points[2].gl_Position.w * s);

		switch (prg.vs2fs[0]) {
			case AttributeType::EMPTY:
				break;
			case AttributeType::FLOAT:
				inFragment.attributes[0].v1 = primitive.points[0].attributes[0].v1 * lambdaA + primitive.points[1].attributes[0].v1 * lambdaB + primitive.points[2].attributes[0].v1 * lambdaC;
				break;
			case AttributeType::VEC2:

				inFragment.attributes[0].v2 = primitive.points[0].attributes[0].v2 * lambdaA + primitive.points[1].attributes[0].v2 * lambdaB + primitive.points[2].attributes[0].v2 * lambdaC;
				break;
			case AttributeType::VEC3:
				inFragment.attributes[0].v3 = primitive.points[0].attributes[0].v3 * lambdaA + primitive.points[1].attributes[0].v3 * lambdaB + primitive.points[2].attributes[0].v3 * lambdaC;
				break;
			case AttributeType::VEC4:
				inFragment.attributes[0].v4 = primitive.points[0].attributes[0].v4 * lambdaA + primitive.points[1].attributes[0].v4 * lambdaB + primitive.points[2].attributes[0].v4 * lambdaC;
				break;
		}
	
}


void runVertexAssembly(InVertex &in, VertexArray &vao, uint32_t shaderInvocation) {
	in.gl_VertexID = computeVertexID(vao, shaderInvocation);
	readAttributes(in, vao);
}

void primitiveAssembly(GPUContext& ctx, Primitive &primitive, uint32_t t) {
	for (int i = 0; i < 3; i++) {
		InVertex in;
		runVertexAssembly(in, ctx.vao, t - 2 + i);
		ctx.prg.vertexShader(primitive.points[i], in, ctx.prg.uniforms);
	}
}

void normalizedDeviceCoordinates(Primitive &primitive) {
	for (int i = 0; i < 3; i++) {
		primitive.points[i].gl_Position.x = primitive.points[i].gl_Position.x / primitive.points[i].gl_Position.w;
		primitive.points[i].gl_Position.y = primitive.points[i].gl_Position.y / primitive.points[i].gl_Position.w;
		primitive.points[i].gl_Position.z = primitive.points[i].gl_Position.z / primitive.points[i].gl_Position.w;
	}
}

void viewportTransformation(Primitive &primitive, GPUContext &ctx) {
	for (int i = 0; i < 3; i++) {
		primitive.points[i].gl_Position.x = (primitive.points[i].gl_Position.x * 0.5 + 0.5) * ctx.frame.width;
		primitive.points[i].gl_Position.y = (primitive.points[i].gl_Position.y * 0.5 + 0.5) * ctx.frame.height;
	}
}

bool compare_float(float x, float y) {
	float epsilon = 0.1f;
	if (std::abs(x - y) < epsilon)
		return true; 
	return false; 
}

void getEdges(Primitive& primitive, glm::vec3 edges[]) {
	edges[0] = { primitive.points[1].gl_Position.x - primitive.points[0].gl_Position.x,  primitive.points[1].gl_Position.y - primitive.points[0].gl_Position.y, 0 };
	edges[1] = { primitive.points[2].gl_Position.x - primitive.points[1].gl_Position.x,  primitive.points[2].gl_Position.y - primitive.points[1].gl_Position.y, 0 };
	edges[2] = { primitive.points[0].gl_Position.x - primitive.points[2].gl_Position.x,  primitive.points[0].gl_Position.y - primitive.points[2].gl_Position.y, 0 };
}


bool inTriangleBarycentric(Primitive& primitive, glm::vec3 edges[],  float x, float y, float &deltaA, float& deltaB, float& deltaC) {

	glm::vec3 con[3];
	con[0] = { x - primitive.points[0].gl_Position.x, y - primitive.points[0].gl_Position.y, 0};
	con[1] = { x - primitive.points[1].gl_Position.x, y - primitive.points[1].gl_Position.y, 0};
	con[2] = { x - primitive.points[2].gl_Position.x, y - primitive.points[2].gl_Position.y, 0};
	float A = glm::cross(edges[0], con[0]).z;
	float B = glm::cross(edges[1], con[1]).z;
	float C = glm::cross(edges[2], con[2]).z;

	float D = glm::cross(edges[0], edges[1]).z;

	deltaA = B / std::abs(D);
	deltaB = C / std::abs(D);
	deltaC = A / std::abs(D);


	if (compare_float(deltaA + deltaB + deltaC, 1.0) == false || deltaA < 0.0f || deltaB < 0.0f || deltaC < 0.0f)
		return false;

	return true;
}

bool counterClockWise(glm::vec3 edges[]) {
	if(glm::cross(edges[0], edges[1]).z >= 0 && glm::cross(edges[1], edges[2]).z >= 0 && glm::cross(edges[2], edges[0]).z >= 0)
		return false;
	return true;

}

void clampColor(OutFragment& outFragment, float min, float max) {
	
	if(!compare_float(outFragment.gl_FragColor.x, 1.0) && !compare_float(outFragment.gl_FragColor.y, 1.0) && !compare_float(outFragment.gl_FragColor.z, 1.0))
	outFragment.gl_FragColor.x = std::max(std::min(outFragment.gl_FragColor.x, max), min);
	outFragment.gl_FragColor.x = outFragment.gl_FragColor.x * 255;

	outFragment.gl_FragColor.y = std::max(std::min(outFragment.gl_FragColor.y, max), min);
	outFragment.gl_FragColor.y = outFragment.gl_FragColor.y * 255;

	outFragment.gl_FragColor.z = std::max(std::min(outFragment.gl_FragColor.z, max), min);
	outFragment.gl_FragColor.z = outFragment.gl_FragColor.z * 255;
}

void perFragmentOperations(Frame& frame, uint32_t x, uint32_t y, OutFragment& outFragment, float depth) {
	uint32_t pos = (frame.height)* y + x;
	if (frame.depth[pos] > depth) {
		if (outFragment.gl_FragColor.w > 0.5f) {
			frame.depth[pos] = depth;
		}
		frame.color[4 * pos] = frame.color[4 * pos] * (1.0f - outFragment.gl_FragColor.w) + outFragment.gl_FragColor.x * outFragment.gl_FragColor.w;
		frame.color[4 * pos + 1] = frame.color[4 * pos + 1] * (1.0f - outFragment.gl_FragColor.w) + outFragment.gl_FragColor.y * outFragment.gl_FragColor.w;
		frame.color[4 * pos + 2] = frame.color[4 * pos + 2] * (1.0f - outFragment.gl_FragColor.w) + outFragment.gl_FragColor.z * outFragment.gl_FragColor.w;
	}
}

void rasterizeTriangle(GPUContext &ctx, Primitive &primitive) {
	
	glm::vec3 edges[3];
	getEdges(primitive, edges);

	if (ctx.backfaceCulling == true && counterClockWise(edges))
		return;

	bool cmp = !ctx.backfaceCulling && counterClockWise(edges) ? false : true;

	for (uint32_t x = 0; x < ctx.frame.width; x++) {
		for (uint32_t y = 0; y < ctx.frame.height; y++) {
			float deltaA, deltaB, deltaC;
			if (inTriangleBarycentric(primitive, edges, x + 0.5f, y + 0.5f, deltaA, deltaB, deltaC) == cmp) {
				InFragment inFragment;
				inFragment.gl_FragCoord.x = x + 0.5f;
				inFragment.gl_FragCoord.y = y + 0.5f;
				inFragment.gl_FragCoord.z = deltaA*primitive.points[0].gl_Position.z + deltaB*primitive.points[1].gl_Position.z + deltaC*primitive.points[2].gl_Position.z;
				interpolateAttributes(primitive, ctx.prg, inFragment, deltaA, deltaB, deltaC); 

				OutFragment outFragment;
				ctx.prg.fragmentShader(outFragment, inFragment, ctx.prg.uniforms);

				clampColor(outFragment, 0, 1);
				perFragmentOperations(ctx.frame, x, y, outFragment, inFragment.gl_FragCoord.z);
			}
		}
	}
}

void performeClipping(Clipped& clipped, Primitive primitive) {
	//uz netusim jak dal...
	clipped.push_back(primitive);
}

//! [drawImpl]
void drawImpl(GPUContext &ctx,uint32_t nofVertices){
	
	for (uint32_t i = 2; i < nofVertices; i+=3) {
		Primitive primitive;
		primitiveAssembly(ctx, primitive, i);

		//Clipped clipped;
		//performeClipping(clipped, primitive);

		normalizedDeviceCoordinates(primitive);
		viewportTransformation(primitive, ctx);
		rasterizeTriangle(ctx, primitive);
	}
  /// \todo Tato funkce vykreslí trojúhelníky podle daného nastavení.<br>
  /// ctx obsahuje aktuální stav grafické karty.
  /// Parametr "nofVertices" obsahuje počet vrcholů, který by se měl vykreslit (3 pro jeden trojúhelník).<br>
  /// Bližší informace jsou uvedeny na hlavní stránce dokumentace.
}
//! [drawImpl]

/**
 * @brief This function reads color from texture.
 *
 * @param texture texture
 * @param uv uv coordinates
 *
 * @return color 4 floats
 */
glm::vec4 read_texture(Texture const&texture,glm::vec2 uv){
  if(!texture.data)return glm::vec4(0.f);
  auto uv1 = glm::fract(uv);
  auto uv2 = uv1*glm::vec2(texture.width-1,texture.height-1)+0.5f;
  auto pix = glm::uvec2(uv2);
  //auto t   = glm::fract(uv2);
  glm::vec4 color = glm::vec4(1.f,0.f,0.f,1.f);
  for(uint32_t c=0;c<texture.channels;++c)
	color[c] = texture.data[(pix.y*texture.width+pix.x)*texture.channels+c]/255.f;
  return color;
}

/**
 * @brief This function clears framebuffer.
 *
 * @param ctx GPUContext
 * @param r red channel
 * @param g green channel
 * @param b blue channel
 * @param a alpha channel
 */
void clear(GPUContext&ctx,float r,float g,float b,float a){
  auto&frame = ctx.frame;
  auto const nofPixels = frame.width * frame.height;
  for(size_t i=0;i<nofPixels;++i){
	frame.depth[i] = 10e10f;
	frame.color[i*4+0] = static_cast<uint8_t>(glm::min(r*255.f,255.f));
	frame.color[i*4+1] = static_cast<uint8_t>(glm::min(g*255.f,255.f));
	frame.color[i*4+2] = static_cast<uint8_t>(glm::min(b*255.f,255.f));
	frame.color[i*4+3] = static_cast<uint8_t>(glm::min(a*255.f,255.f));
  }
}

