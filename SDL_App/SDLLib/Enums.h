/********************************/
/* SDLLib Emscripten Framework  */
/* Copyright © 2016             */
/* Author: Wong Shao Voon       */
/********************************/

#pragma once

enum class Stage
{
	FirstBkgd,
	Stencil1,
	SecondBkgd,
	Stencil2,
	LastBkgd
};

enum class Precision
{
	lowp,
	mediump,
	highp
};