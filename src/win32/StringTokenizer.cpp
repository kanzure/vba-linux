// StringTokenizer.cpp: implementation of the StringTokenizer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "StringTokenizer.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

StringTokenizer::StringTokenizer(CString str, CString del)
{
	m_right = str;
	m_delim = del;
}

StringTokenizer::~StringTokenizer()
{}

const char *StringTokenizer::next()
{
	int index = m_right.FindOneOf(m_delim);

	while (index == 0)
	{
		m_right = m_right.Right(m_right.GetLength()-1);
		index   = m_right.FindOneOf(m_delim);
	}
	if (index == -1)
	{
		if (m_right.IsEmpty())
			return NULL;
		m_token = m_right;
		m_right.Empty();
		return m_token;
	}

	m_token = m_right.Left(index);
	m_right = m_right.Right(m_right.GetLength()-(1+index));

	return m_token;
}

