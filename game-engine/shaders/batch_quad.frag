#version 330 core
out vec4 o_color;

in vec4 v_color;
in vec2 v_uvs;
flat in int v_texture_slot;

uniform sampler2D texture_slot_0;
uniform sampler2D texture_slot_1;
uniform sampler2D texture_slot_2;
uniform sampler2D texture_slot_3;
uniform sampler2D texture_slot_4;
uniform sampler2D texture_slot_5;
uniform sampler2D texture_slot_6;
uniform sampler2D texture_slot_7;
uniform sampler2D texture_slot_8;
uniform sampler2D texture_slot_9;
uniform sampler2D texture_slot_10;
uniform sampler2D texture_slot_11;
uniform sampler2D texture_slot_12;
uniform sampler2D texture_slot_13;
uniform sampler2D texture_slot_14;
uniform sampler2D texture_slot_15;
uniform sampler2D texture_slot_16;
uniform sampler2D texture_slot_17;
uniform sampler2D texture_slot_18;
uniform sampler2D texture_slot_19;
uniform sampler2D texture_slot_20;
uniform sampler2D texture_slot_21;
uniform sampler2D texture_slot_22;
uniform sampler2D texture_slot_23;
uniform sampler2D texture_slot_24;
uniform sampler2D texture_slot_25;
uniform sampler2D texture_slot_26;
uniform sampler2D texture_slot_27;
uniform sampler2D texture_slot_28;
uniform sampler2D texture_slot_29;
uniform sampler2D texture_slot_30;
uniform sampler2D texture_slot_31;

void main() {
	switch (v_texture_slot) {
	case 0: o_color = texture(texture_slot_0, v_uvs) * v_color; break;
	case 1: o_color = texture(texture_slot_1, v_uvs) * v_color; break;
	case 2: o_color = texture(texture_slot_2, v_uvs) * v_color; break;
	case 3: o_color = texture(texture_slot_3, v_uvs) * v_color; break;
	case 4: o_color = texture(texture_slot_4, v_uvs) * v_color; break;
	case 5: o_color = texture(texture_slot_5, v_uvs) * v_color; break;
	case 6: o_color = texture(texture_slot_6, v_uvs) * v_color; break;
	case 7: o_color = texture(texture_slot_7, v_uvs) * v_color; break;
	case 8: o_color = texture(texture_slot_8, v_uvs) * v_color; break;
    case 9: o_color = texture(texture_slot_9, v_uvs) * v_color; break;
    case 10: o_color = texture(texture_slot_10, v_uvs) * v_color; break;
    case 11: o_color = texture(texture_slot_11, v_uvs) * v_color; break;
    case 12: o_color = texture(texture_slot_12, v_uvs) * v_color; break;
    case 13: o_color = texture(texture_slot_13, v_uvs) * v_color; break;
    case 14: o_color = texture(texture_slot_14, v_uvs) * v_color; break;
    case 15: o_color = texture(texture_slot_15, v_uvs) * v_color; break;
	case 16: o_color = texture(texture_slot_16, v_uvs) * v_color; break;
	case 17: o_color = texture(texture_slot_17, v_uvs) * v_color; break;
	case 18: o_color = texture(texture_slot_18, v_uvs) * v_color; break;
	case 19: o_color = texture(texture_slot_19, v_uvs) * v_color; break;
	case 20: o_color = texture(texture_slot_20, v_uvs) * v_color; break;
	case 21: o_color = texture(texture_slot_21, v_uvs) * v_color; break;
	case 22: o_color = texture(texture_slot_22, v_uvs) * v_color; break;
	case 23: o_color = texture(texture_slot_23, v_uvs) * v_color; break;
	case 24: o_color = texture(texture_slot_24, v_uvs) * v_color; break;
	case 25: o_color = texture(texture_slot_25, v_uvs) * v_color; break;
	case 26: o_color = texture(texture_slot_26, v_uvs) * v_color; break;
	case 27: o_color = texture(texture_slot_27, v_uvs) * v_color; break;
	case 28: o_color = texture(texture_slot_28, v_uvs) * v_color; break;
	case 29: o_color = texture(texture_slot_29, v_uvs) * v_color; break;
	case 30: o_color = texture(texture_slot_30, v_uvs) * v_color; break;
	case 31: o_color = texture(texture_slot_31, v_uvs) * v_color; break;
	}
}