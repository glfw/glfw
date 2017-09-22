#pragma once
#include "IModel.h"

/** 
*	A simple cube model that is easy to render
*/
class CubeModel : public IModel
{
public:
	// Methods
	CubeModel(const std::string& sVSFileName, const std::string& sFSFileName, float side);

	void	render() const;

protected:
	// Fields
	float m_side;
};

