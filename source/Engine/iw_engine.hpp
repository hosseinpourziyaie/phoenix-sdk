#pragma once
#include <imgui_core.hpp>

namespace IW
{
	//#define R_AddCmdDrawText(TXT, MC, F, X, Y, XS, YS, R, C, S) \
	//	imgui::draw_text(TXT, X, Y, C, F, 16.0f)
	//
	//#define R_AddCmdDrawStretchPic(X, Y, W, H, A5, A6, A7, A8, C, A10) \
	//	imgui::draw_rect(X, Y, W, H, C)
	//
	//#define R_TextWidth(TXT, MC, F) \
	//	ImGui::CalcTextSize(TXT)
	//
	//#define Cbuf_AddText(A1, A2, A3) NULL

	struct CmdFunc
	{
		CmdFunc* next;
		const char* name;
		void(__cdecl* function)();
	};

	typedef struct {
		byte* data;
		int		maxsize;
		int		cmdsize;
	} CmdText;

	typedef struct 
	{
		const char* fontName;
		float pixelHeight;
		ImFont* fontHandle;
	} font_s;


	extern void R_AddCmdDrawTextWithCursor(std::string text, int maxChars, font_s* font, float x, float y, float xScale, float yScale, float rotation, float* color, int style, int cursor_pos, char cursor_char);
	extern void R_AddCmdDrawText(const char* text, int maxChars, font_s* font, float x, float y, float xScale, float yScale, float rotation, float* color, int style);
	extern void R_AddCmdDrawStretchPic(float x, float y, float w, float h, float s0, float t0, float s1, float t1, float* color, void* material);
	extern int R_TextWidth(const char* text, int maxChars, font_s* font);
}
