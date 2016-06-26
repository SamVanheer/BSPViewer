#include <cassert>
#include <cstdio>
#include <cstring>
#include <memory>

#include "CBaseShader.h"

CBaseShader* CBaseShader::m_pHead = nullptr;

CBaseShader::CBaseShader()
{
	m_pNext = m_pHead;
	m_pHead = this;
}