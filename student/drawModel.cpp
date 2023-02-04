/*!
 * @file
 * @brief This file contains functions for model rendering
 *
 * @author Tomáš Milet, imilet@fit.vutbr.cz
 */
#include <student/drawModel.hpp>
#include <student/gpu.hpp>


void drawNode(GPUContext& ctx, Node const& node, Model const& model, glm::mat4 const& prubeznaMatice) {
	if (node.mesh >= 0) {
		Mesh const& mesh = model.meshes[node.mesh];
		
		ctx.prg.uniforms.uniform[1].m4 = prubeznaMatice * node.modelMatrix;
		ctx.prg.uniforms.uniform[2].m4 = glm::transpose(glm::inverse(ctx.prg.uniforms.uniform[1].m4));

		ctx.vao.vertexAttrib[0] = mesh.position;
		ctx.vao.indexType = mesh.indexType;

		ctx.vao.vertexAttrib[0] = mesh.position;
		ctx.vao.vertexAttrib[1] = mesh.normal;
		ctx.vao.vertexAttrib[2] = mesh.texCoord;
		
		if (mesh.diffuseTexture >= 0) {
			ctx.prg.uniforms.textures[0] = model.textures[mesh.diffuseTexture];
			ctx.vao.vertexAttrib[2] = mesh.texCoord;
		}
		else {
			ctx.prg.uniforms.textures[0] = Texture{};
			ctx.prg.uniforms.uniform[5].v4 = mesh.diffuseColor;
			ctx.prg.uniforms.uniform[6].v1 = 0.0f;
		}

		draw(ctx, mesh.nofIndices);
	}
	for (int i = 0; i < node.children.size(); i++) {
		drawNode(ctx, node.children[i], model, prubeznaMatice * node.modelMatrix);
	}
}

/**
 * @brief This function renders a model
 *
 * @param ctx GPUContext
 * @param model model structure
 * @param proj projection matrix
 * @param view view matrix
 * @param light light position
 * @param camera camera position (unused)
 */
//! [drawModel]
void drawModel(GPUContext&ctx,Model const&model,glm::mat4 const&proj,glm::mat4 const&view,glm::vec3 const&light,glm::vec3 const&camera){
	
	ctx.backfaceCulling = false;
	ctx.prg.uniforms.uniform[0].m4 = proj * view;
	ctx.prg.uniforms.uniform[3].v3 = light;
	ctx.prg.uniforms.uniform[4].v3 = camera;
	ctx.prg.vs2fs[0] = AttributeType::VEC3;
	ctx.prg.vs2fs[1] = AttributeType::VEC3;
	ctx.prg.vs2fs[2] = AttributeType::VEC2;


	ctx.prg.vertexShader = drawModel_vertexShader;
	ctx.prg.fragmentShader = drawModel_fragmentShader;

	glm::mat4 jednotkovaMatrice = glm::mat4(1.f);
	for (size_t i = 0; i < model.roots.size(); ++i)
		drawNode(ctx, model.roots[i], model, jednotkovaMatrice);
	
  
  ;
  /// \todo Tato funkce vykreslí model.<br>
  /// Vaším úkolem je správně projít model a vykreslit ho pomocí funkce draw (nevolejte drawImpl, je to z důvodu testování).
  /// Bližší informace jsou uvedeny na hlavní stránce dokumentace.
}
//! [drawModel]

/**
 * @brief This function represents vertex shader of texture rendering method.
 *
 * @param outVertex output vertex
 * @param inVertex input vertex
 * @param uniforms uniform variables
 */
//! [drawModel_vs]
void drawModel_vertexShader(OutVertex&outVertex,InVertex const&inVertex,Uniforms const&uniforms){
	outVertex.attributes[0].v3 = uniforms.uniform[1].m4 * glm::vec4(inVertex.attributes[0].v3, 1.0f);
	outVertex.attributes[1].v3 = uniforms.uniform[2].m4 * glm::vec4(inVertex.attributes[1].v3, 1.0f);
	outVertex.attributes[2].v2 = inVertex.attributes[2].v2;
	outVertex.gl_Position = uniforms.uniform[0].m4 * glm::vec4(inVertex.attributes[0].v3, 1.0f);
  /// \todo Tato funkce reprezentujte vertex shader.<br>
  /// Vaším úkolem je správně trasnformovat vrcholy modelu.
  /// Bližší informace jsou uvedeny na hlavní stránce dokumentace.
}
//! [drawModel_vs]

/**
 * @brief This functionrepresents fragment shader of texture rendering method.
 *
 * @param outFragment output fragment
 * @param inFragment input fragment
 * @param uniforms uniform variables
 */
//! [drawModel_fs]
void drawModel_fragmentShader(OutFragment&outFragment,InFragment const&inFragment,Uniforms const&uniforms){
	

  /// \todo Tato funkce reprezentujte fragment shader.<br>
  /// Vaším úkolem je správně obarvit fragmenty a osvětlit je pomocí lambertova osvětlovacího modelu.
  /// Bližší informace jsou uvedeny na hlavní stránce dokumentace.
}
//! [drawModel_fs]

