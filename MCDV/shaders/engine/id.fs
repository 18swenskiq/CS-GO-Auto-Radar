#version 330 core
layout (location = 0) out uint frag;

uniform uint id;

void main()
{
	frag = id;
}