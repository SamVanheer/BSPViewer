#ifndef GL_CBASESHADER_H
#define GL_CBASESHADER_H

#include <vector>

#include <gl/glew.h>

#include <glm/mat4x4.hpp>

#include "GLUtil.h"

class CBaseEntity;

#define SHADER_BASE_DIR "shaders/"
#define SHADER_VERTEX_EXT ".vtx"
#define SHADER_FRAG_EXT ".frag"

/**
*	Attribute types.
*	This must be kept in sync with the sizes list.
*/
enum class AttributeType
{
	INTEGER,
	FLOAT,
	VEC2,
	VEC3,
	VEC4,
	MAT4X4,

	/**
	*	A sampler texture.
	*/
	SAMPLER_TEXTURE,

	NUM_TYPES
};

/**
*	Represents a single attribute.
*/
class CBaseShaderAttribute
{
public:
	static const size_t INVALID_INDEX = -1;

public:
	CBaseShaderAttribute( const char* const pszName, const AttributeType type, const bool bIsVarying )
		: m_pszName( pszName )
		, m_Type( type )
		, m_bIsVarying( bIsVarying )
	{
	}

	const char* GetName() const { return m_pszName; }

	AttributeType GetType() const { return m_Type; }

	/**
	*	@return Whether this attribute is varying (true) or uniform (false).
	*/
	bool IsVarying() const { return m_bIsVarying; }

	size_t GetIndex() const { return m_uiIndex; }

	operator size_t() const { return m_uiIndex; }

private:
	/**
	*	Name of the attribute as defined in the shader.
	*/
	const char* const m_pszName;
	const AttributeType m_Type;

	const bool m_bIsVarying;

protected:
	size_t m_uiIndex = INVALID_INDEX;
};

class CBaseShaderOutput
{
public:
	CBaseShaderOutput( const char* const pszName )
		: m_pszName( pszName )
	{
	}

	const char* GetName() const { return m_pszName; }

private:
	const char* const m_pszName;
};

#define BEGIN_SHADER( shaderName )											\
namespace shaderName														\
{																			\
	static const char* const g_pszName = #shaderName;						\
	class CShaderAttribute;													\
																			\
	static std::vector<CShaderAttribute*> g_Attributes;						\
																			\
	class CShaderAttribute final : public CBaseShaderAttribute				\
	{																		\
	public:																	\
		CShaderAttribute( 													\
			const char* const pszName, const AttributeType type )			\
			: CBaseShaderAttribute( pszName, type, true )					\
		{																	\
			m_uiIndex = g_Attributes.size();								\
																			\
			g_Attributes.push_back( this );									\
		}																	\
	};																		\
																			\
	class CShaderUniform;													\
																			\
	static std::vector<CShaderUniform*> g_Uniforms;							\
																			\
	class CShaderUniform final : public CBaseShaderAttribute				\
	{																		\
	public:																	\
		CShaderUniform( 													\
			const char* const pszName, const AttributeType type )			\
			: CBaseShaderAttribute( pszName, type, false )					\
		{																	\
			m_uiIndex = g_Uniforms.size();									\
																			\
			g_Uniforms.push_back( this );									\
		}																	\
	};																		\
																			\
	class CShaderOutput;													\
																			\
	static std::vector<CShaderOutput*> g_Outputs;							\
																			\
	class CShaderOutput final : public CBaseShaderOutput					\
	{																		\
	public:																	\
		CShaderOutput( const char* const pszName )							\
			: CBaseShaderOutput( pszName )									\
		{																	\
			g_Outputs.push_back( this );									\
		}																	\
	};

#define END_SHADER()		\
	} g_Instance;			\
}

#define SHADER_ATTRIB( name, type ) static CShaderAttribute name( #name, AttributeType::type );

#define SHADER_UNIFORM( name, type ) static CShaderUniform name( #name, AttributeType::type );

#define SHADER_OUTPUT( name ) static CShaderOutput name( #name );

#define BEGIN_SHADER_ATTRIBS()

#define END_SHADER_ATTRIBS() BEGIN_SHADER_CLASS()

#define BEGIN_SHADER_CLASS()													\
class CShader : public CBaseShader												\
{																				\
public:																			\
																				\
	CShader()																	\
		: CBaseShader()															\
	{																			\
	}																			\
																				\
	const char* GetName() const override										\
	{																			\
		return g_pszName;														\
	}																			\
																				\
	size_t GetNumAttributes() const override									\
	{																			\
		return g_Attributes.size();												\
	}																			\
																				\
	CBaseShaderAttribute* GetAttribute( const size_t uiIndex ) const override	\
	{																			\
		return g_Attributes[ uiIndex ];											\
	}																			\
																				\
	size_t GetNumUniforms() const override										\
	{																			\
		return g_Uniforms.size();												\
	}																			\
																				\
	CBaseShaderAttribute* GetUniform( const size_t uiIndex ) const override		\
	{																			\
		return g_Uniforms[ uiIndex ];											\
	}																			\
																				\
	size_t GetNumOutputs() const override										\
	{																			\
		return g_Outputs.size();												\
	}																			\
																				\
	CBaseShaderOutput* GetOutput( const size_t uiIndex ) const override			\
	{																			\
		return g_Outputs[ uiIndex ];											\
	}

#define SHADER_ACTIVATE void Activate( CShaderInstance* pInstance, const CBaseEntity* pEntity ) override

#define SHADER_DRAW void OnDraw( CShaderInstance* pInstance, const size_t uiNumVerts ) override

class CShaderInstance;

/**
*	Base class for shaders.
*/
class CBaseShader
{
public:
	CBaseShader();

	/**
	*	@return First shader in the global list.
	*/
	static CBaseShader* GetHead() { return m_pHead; }

	/**
	*	@return Next shader in the global list.
	*/
	CBaseShader* GetNext() const { return m_pNext; }

	virtual const char* GetName() const = 0;

	virtual size_t GetNumAttributes() const = 0;
	virtual CBaseShaderAttribute* GetAttribute( const size_t uiIndex ) const = 0;

	virtual size_t GetNumUniforms() const = 0;
	virtual CBaseShaderAttribute* GetUniform( const size_t uiIndex ) const = 0;

	virtual size_t GetNumOutputs() const = 0;
	virtual CBaseShaderOutput* GetOutput( const size_t uiIndex ) const = 0;

	virtual void Activate( CShaderInstance* pInstance, const CBaseEntity* pEntity ) {}

	virtual void OnDraw( CShaderInstance* pInstance, const size_t uiNumVerts ) = 0;

private:
	static CBaseShader* m_pHead;
	CBaseShader* m_pNext;
};

#endif //GL_CBASESHADER_H