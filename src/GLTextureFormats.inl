
typedef struct {
	bonobo::TextureType	type;
	size_t			components;
	vec4i			componentBits;
	GLenum			iformat;
	size_t			totalByteSize;
} TextureFormatPair;

static const TextureFormatPair glTextureFormatMap[] = {
	{bonobo::TEXTURE_UNORM			, 1		, vec4i::Make(8 , 0 , 0 , 0 )		, GL_R8				, 1		},
	{bonobo::TEXTURE_SNORM			, 1		, vec4i::Make(8 , 0 , 0 , 0 )		, GL_R8_SNORM		, 1		},
	{bonobo::TEXTURE_UNORM			, 1		, vec4i::Make(16, 0 , 0 , 0 )		, GL_R16			, 2		},
	{bonobo::TEXTURE_SNORM			, 1		, vec4i::Make(16, 0 , 0 , 0 )		, GL_R16_SNORM		, 2		},
	{bonobo::TEXTURE_UNORM			, 2		, vec4i::Make(8 , 8 , 0 , 0 )		, GL_RG8			, 2		},
	{bonobo::TEXTURE_SNORM			, 2		, vec4i::Make(8 , 8 , 0 , 0 )		, GL_RG8_SNORM		, 2		},
	{bonobo::TEXTURE_UNORM			, 2		, vec4i::Make(16, 16, 0 , 0 )		, GL_RG16			, 4		},
	{bonobo::TEXTURE_SNORM			, 2		, vec4i::Make(16, 16, 0 , 0 )		, GL_RG16_SNORM		, 4		},
	{bonobo::TEXTURE_UNORM			, 3		, vec4i::Make(3 , 3 , 2 , 0 )		, GL_R3_G3_B2		, 1		},
	{bonobo::TEXTURE_UNORM			, 3		, vec4i::Make(4 , 4 , 4 , 0 )		, GL_RGB4			, 2		}, // TODO
	{bonobo::TEXTURE_UNORM			, 3		, vec4i::Make(5 , 5 , 5 , 0 )		, GL_RGB5			, 2		}, // TODO
	{bonobo::TEXTURE_UNORM			, 3		, vec4i::Make(8 , 8 , 8 , 0 )		, GL_RGB8			, 3		},
	{bonobo::TEXTURE_SNORM			, 3		, vec4i::Make(8 , 8 , 8 , 0 )		, GL_RGB8_SNORM		, 3		},
	{bonobo::TEXTURE_UNORM			, 3		, vec4i::Make(10, 10, 10, 0 )		, GL_RGB10			, 4		},
	{bonobo::TEXTURE_UNORM			, 3		, vec4i::Make(12, 12, 12, 0 )		, GL_RGB12			, 5		}, // TODO
	{bonobo::TEXTURE_UNORM			, 3		, vec4i::Make(16, 16, 16, 0 )		, GL_RGB16			, 6		},
	{bonobo::TEXTURE_SNORM			, 3		, vec4i::Make(16, 16, 16, 0 )		, GL_RGB16_SNORM	, 6		},
	{bonobo::TEXTURE_UNORM			, 4		, vec4i::Make(2 , 2 , 2 , 2 )		, GL_RGBA2			, 1		},
	{bonobo::TEXTURE_UNORM			, 4		, vec4i::Make(4 , 4 , 4 , 4 )		, GL_RGBA4			, 2		},
	{bonobo::TEXTURE_UNORM			, 4		, vec4i::Make(5 , 5 , 5 , 1 )		, GL_RGB5_A1		, 2		},
	{bonobo::TEXTURE_UNORM			, 4		, vec4i::Make(8 , 8 , 8 , 8 )		, GL_RGBA8			, 4		},
	{bonobo::TEXTURE_SNORM			, 4		, vec4i::Make(8 , 8 , 8 , 8 )		, GL_RGBA8_SNORM	, 4		},
	{bonobo::TEXTURE_UNORM			, 4		, vec4i::Make(10, 10, 10, 2 )		, GL_RGB10_A2		, 4		},
	{bonobo::TEXTURE_UINT			, 4		, vec4i::Make(10, 10, 10, 2 )		, GL_RGB10_A2UI		, 4		},
	{bonobo::TEXTURE_UNORM			, 4		, vec4i::Make(12, 12, 12, 12)		, GL_RGBA12			, 6		},
	{bonobo::TEXTURE_UNORM			, 4		, vec4i::Make(16, 16, 16, 16)		, GL_RGBA16			, 8		},
	{bonobo::TEXTURE_SNORM			, 4		, vec4i::Make(16, 16, 16, 16)		, GL_RGBA16_SNORM	, 8		},
	{bonobo::TEXTURE_FLOAT			, 1		, vec4i::Make(16, 0 , 0 , 0 )		, GL_R16F			, 2		},
	{bonobo::TEXTURE_FLOAT			, 2		, vec4i::Make(16, 16, 0 , 0 )		, GL_RG16F			, 4		},
	{bonobo::TEXTURE_FLOAT			, 3		, vec4i::Make(16, 16, 16, 0 )		, GL_RGB16F			, 6		},
	{bonobo::TEXTURE_FLOAT			, 4		, vec4i::Make(16, 16, 16, 16)		, GL_RGBA16F		, 8		},
	{bonobo::TEXTURE_FLOAT			, 1		, vec4i::Make(32, 0 , 0 , 0 )		, GL_R32F			, 4		},
	{bonobo::TEXTURE_FLOAT			, 2		, vec4i::Make(32, 32, 0 , 0 )		, GL_RG32F			, 8		},
	{bonobo::TEXTURE_FLOAT			, 3		, vec4i::Make(32, 32, 32, 0 )		, GL_RGB32F			, 12	},
	{bonobo::TEXTURE_FLOAT			, 4		, vec4i::Make(32, 32, 32, 32)		, GL_RGBA32F		, 16	},
	{bonobo::TEXTURE_FLOAT			, 3		, vec4i::Make(11, 11, 10, 0 )		, GL_R11F_G11F_B10F	, 4		},
	{bonobo::TEXTURE_INT			, 1		, vec4i::Make(8 , 0 , 0 , 0 )		, GL_R8I			, 1		},
	{bonobo::TEXTURE_UINT			, 1		, vec4i::Make(8 , 0 , 0 , 0 )		, GL_R8UI			, 1		},
	{bonobo::TEXTURE_INT			, 1		, vec4i::Make(16, 0 , 0 , 0 )		, GL_R16I			, 2		},
	{bonobo::TEXTURE_UINT			, 1		, vec4i::Make(16, 0 , 0 , 0 )		, GL_R16UI			, 2		},
	{bonobo::TEXTURE_INT			, 1		, vec4i::Make(32, 0 , 0 , 0 )		, GL_R32I			, 4		},
	{bonobo::TEXTURE_UINT			, 1		, vec4i::Make(32, 0 , 0 , 0 )		, GL_R32UI			, 4		},
	{bonobo::TEXTURE_INT			, 2		, vec4i::Make(8 , 8 , 0 , 0 )		, GL_RG8I			, 2		},
	{bonobo::TEXTURE_UINT			, 2		, vec4i::Make(8 , 8 , 0 , 0 )		, GL_RG8UI			, 2		},
	{bonobo::TEXTURE_INT			, 2		, vec4i::Make(16, 16, 0 , 0 )		, GL_RG16I			, 4		},
	{bonobo::TEXTURE_UINT			, 2		, vec4i::Make(16, 16, 0 , 0 )		, GL_RG16UI			, 4		},
	{bonobo::TEXTURE_INT			, 2		, vec4i::Make(32, 32, 0 , 0 )		, GL_RG32I			, 8		},
	{bonobo::TEXTURE_UINT			, 2		, vec4i::Make(32, 32, 0 , 0 )		, GL_RG32UI			, 8		},
	{bonobo::TEXTURE_INT			, 3		, vec4i::Make(8 , 8 , 8 , 0 )		, GL_RGB8I			, 3		},
	{bonobo::TEXTURE_UINT			, 3		, vec4i::Make(8 , 8 , 8 , 0 )		, GL_RGB8UI			, 3		},
	{bonobo::TEXTURE_INT			, 3		, vec4i::Make(16, 16, 16, 0 )		, GL_RGB16I			, 6		},
	{bonobo::TEXTURE_UINT			, 3		, vec4i::Make(16, 16, 16, 0 )		, GL_RGB16UI		, 6		},
	{bonobo::TEXTURE_INT			, 3		, vec4i::Make(32, 32, 32, 0 )		, GL_RGB32I			, 12	},
	{bonobo::TEXTURE_UINT			, 3		, vec4i::Make(32, 32, 32, 0 )		, GL_RGB32UI		, 12	},
	{bonobo::TEXTURE_INT			, 4		, vec4i::Make(8 , 8 , 8 , 8 )		, GL_RGBA8I			, 4		},
	{bonobo::TEXTURE_UINT			, 4		, vec4i::Make(8 , 8 , 8 , 8 )		, GL_RGBA8UI		, 4		},
	{bonobo::TEXTURE_INT			, 4		, vec4i::Make(16, 16, 16, 16)		, GL_RGBA16I		, 8		},
	{bonobo::TEXTURE_UINT			, 4		, vec4i::Make(16, 16, 16, 16)		, GL_RGBA16UI		, 8		},
	{bonobo::TEXTURE_INT			, 4		, vec4i::Make(32, 32, 32, 32)		, GL_RGBA32I		, 16	},
	{bonobo::TEXTURE_UINT			, 4		, vec4i::Make(32, 32, 32, 32)		, GL_RGBA32UI		, 16	},
																					
	{bonobo::TEXTURE_FLOAT_DEPTH	, 1		, vec4i::Make(32, 0 , 0 , 0 )		, GL_DEPTH_COMPONENT, 4		},
																					
	{bonobo::TEXTURE_TYPE_LAST		, 0		, vec4i::Make(0 , 0 , 0 , 0 )		, GL_R8				, 1		}
};

