#pragma once
#include "IModel.h"

/** 
*	A simple sphere model that is easy to render
*/
class SphereModel : public IModel
{
public:
	// Methods
	SphereModel(const std::string& sVSFileName, const std::string& sFSFileName, float radius, unsigned int meshPrecision);

	void	render() const;

private:
	// Fields
	float			m_radius;
	unsigned int	m_meshPrecision;
};

