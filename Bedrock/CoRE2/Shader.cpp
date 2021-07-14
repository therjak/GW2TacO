#include "Shader.h"

//////////////////////////////////////////////////////////////////////////
// base shader class

CCoreShader::~CCoreShader() {
  SAFEDELETEA(Binary);
  BinaryLength = 0;
}

//////////////////////////////////////////////////////////////////////////
// base pixelshader class

CCorePixelShader::~CCorePixelShader() {}

//////////////////////////////////////////////////////////////////////////
// base geometryshader class

CCoreGeometryShader::~CCoreGeometryShader() {}

//////////////////////////////////////////////////////////////////////////
// base vertexshader class

CCoreVertexShader::~CCoreVertexShader() {}

//////////////////////////////////////////////////////////////////////////
// base Hullshader class

CCoreHullShader::~CCoreHullShader() {}

//////////////////////////////////////////////////////////////////////////
// base Domainshader class

CCoreDomainShader::~CCoreDomainShader() {}

//////////////////////////////////////////////////////////////////////////
// base Computeshader class

CCoreComputeShader::~CCoreComputeShader() {}
