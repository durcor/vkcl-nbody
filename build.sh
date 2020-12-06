#!/bin/sh

build_vkcl()
{
	[ -d "build" ] &&
		rm -r build
	if meson build && ninja -C build; then
		echo "$(tput setaf 2)vkcl built successfully!$(tput sgr0)"
	else
		echo "$(tput setaf 2)Something went wrong while building vkcl!$(tput sgr0)"
		exit 1
	fi
}

build_nbody_test()
{
	[ ! -d "build" ] &&
		build_vkcl

	if g++ nbody.cpp -Iinclude -Lbuild/ -lvkcl -lGLU -lGL -lglut -lvulkan -ldl -fopenmp -o nbody; then
		echo "$(tput setaf 2)n-body test built successfully!$(tput sgr0)"
		echo "You can run the n-body test with: $(tput bold)$(tput setaf 2)./nbody$(tput sgr0)"
	else
		echo "$(tput setaf 1)Something went wrong while building the nbody test!$(tput sgr0)"
		echo "This test requires glut (freeglut), vulkan (vulkan-headers), and openmp (openmp)"
		exit 1
	fi
}

case $1 in
	"vkcl")
		build_vkcl
		;;
	"nbody")
		build_nbody_test
		;;
	*)
		build_vkcl
		build_nbody_test
		;;
esac
