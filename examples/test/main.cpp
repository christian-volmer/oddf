/*

	ODDF - Open Digital Design Framework
	Copyright Advantest Corporation
	
	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 3 of the License, or
	(at your option) any later version.
	
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
	
	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <https://www.gnu.org/licenses/>.

*/

/*

	Test program that performs various arithmetic functions.

*/

#include "../../lib/oddf/src/dfx.h"
#include "../../lib/verilog/verilog.h"

#include <cmath>

namespace b = dfx::blocks;

using dfx::dynfix;
using dfx::sfix;
using dfx::ufix;

namespace b = dfx::blocks;
namespace m = dfx::modules;

template <typename nodeT>
dfx::node<nodeT> FirFilter(dfx::node<nodeT> const &input, std::vector<nodeT> const &coefficients)
{
	std::size_t length = coefficients.size();

	dfx::forward_bus<nodeT> taps((int)length, input.GetDriver()->value);

	taps <<= dfx::join(input, b::Delay(taps.most()));

	return b::FloorCast(input, b::Sum(taps * b::Constant(coefficients.begin(), coefficients.end())));
}

void CordicTrig(dfx::node<double> &x, dfx::node<double> &y, dfx::node<double> &z, int iteration)
{
	int sigma = iteration;
	double e = 0.15915494309189533577 * std::atan(std::pow(2.0, -iteration));

	dfx::node<bool> direction = z > 0.0;

	dfx::node<double> dx, dy, dz;

	dx = y * std::pow(2.0, -sigma);
	dy = x * std::pow(2.0, -sigma);
	dz = b::Constant(e);

	x = b::Decide(direction, x - dx, x + dx);
	y = b::Decide(direction, y + dy, y - dy);
	z = b::Decide(direction, z - dz, z + dz);
}

dfx::node<double> Sine(dfx::node<double> const &angle)
{
	dfx::node<double> x, y, z;

	x = b::Decide(angle < 0, -1.0, 1.0);
	y = b::Constant(0.0);
	z = b::Decide(angle < 0, -0.25 - angle, 0.25 - angle);

	for (int i = 0; i < 60; ++i)
		CordicTrig(x, y, z, i);

	return 0.60725293500888125617 * x;
}

dfx::node<double> Sine2(dfx::node<double> const &angle)
{
	dfx::node<double> angle2 = angle * angle;

	std::vector<double> coefficients = {
		3.1415926535897932217634138190561669,
		-5.1677127800499696915099303597432201,
		2.5501640398773423323554335019782229,
		-0.59926452932077477566294257370817314,
		0.082145886611063405685101391813296613,
		-0.0073704309455414204191782343528268708,
		0.00046630280542945931361708425543669199,
		-0.000021915352956762105029391294819592436,
		7.9520487014634610501804176636616678e-7,
		-2.2948008734778760872766713602519927e-8,
		5.390281677264039046511075415746811e-10,
		-1.0426742312330399460677082620686781e-11,
		1.5139359809711651665461461876581484e-13};

	auto currentCoeff = coefficients.crbegin();
	dfx::node<double> currentValue = b::Constant(*currentCoeff);
	++currentCoeff;

	while (currentCoeff != coefficients.crend())
	{

		currentValue = angle2 * currentValue + (*currentCoeff);
		++currentCoeff;
	}

	return angle * currentValue;
}

int main()
{
	dfx::Design design;

	m::Source<double> source;

	source.Inputs.ReadEnable <<= b::Constant(true);

	source.SetData({0.0, 0.0, 0.5, 1.7, -0.4, -0.9, 0.9, 1.3, -2});

	dfx::debug::Logger.Log("out", FirFilter<double>(source.Outputs.Data, {1.0, 2.0, 3.0, 4.0}));

	dfx::debug::Logger.Log("out2", FirFilter<dynfix>(b::FloorCast<sfix<60, 50>>(source.Outputs.Data), {1.0, 2.0, 3.0, 4.0}));

	dfx::forward_node<double> angle;
	angle <<= b::Delay(angle + 0.01);

	dfx::debug::Logger.Log("angle", angle);
	dfx::debug::Logger.Log("ref", b::Function(angle, [](double x)
											  { return std::sin(6.2831853071795864769 * x); }));
	dfx::debug::Logger.Log("sin", Sine(angle));
	dfx::debug::Logger.Log("diff", Sine(angle) - b::Function(angle, [](double x)
															 { return std::sin(6.2831853071795864769 * x); }));

	dfx::debug::Logger.Log("diff2", Sine2(2*angle) - b::Function(angle, [](double x)
															 { return std::sin(6.2831853071795864769 * x); }));

	dfx::Simulator simulator(design);

	simulator.Run(10);

	dfx::debug::Logger.WriteTable(std::cout);

	/*	dfx::generator::Generator generator(design, std::cout);
		VerilogExporter exporter(generator);

		exporter.Export(".", std::cout);
	*/

	return 0;
}
