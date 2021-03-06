#include "stdafx.h"
#include "HudElements.h"
#include "H2MOD\Modules\OnScreenDebug\OnScreenDebug.h"
#include "H2MOD\Modules\Startup\Startup.h"
#include "Util\Hooks\Hook.h"
#include "H2MOD/Modules/Config/Config.h"
#include "H2MOD/Modules/AdvLobbySettings/AdvLobbySettings.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include "H2MOD/Modules/Utils/Utils.h"
#include "H2MOD/Modules/Console/ConsoleCommands.h"

static bool b_showHUD = true;
static bool b_showFirstPerson = true;
static bool RenderIngameChat() {
	int GameGlobals = *h2mod->GetAddress<int*>(0x482D3C);
	DWORD* GameEngine = (DWORD*)(GameGlobals + 0x8);

	if (H2Config_hide_ingame_chat) {
		PlayerIterator playerIt;
		datum local_player_datum_index = h2mod->get_player_datum_index_from_controller_index(0);
		if (playerIt.get_data_at_index(local_player_datum_index.Index)->is_chatting == 2) {
			extern void hotkeyFuncToggleHideIngameChat();
			hotkeyFuncToggleHideIngameChat();
		}
		return true;
	}

	else if (*GameEngine != 3 && get_game_life_cycle() == life_cycle_in_game) {
		//Enable chat in engine mode and game state mp.
		return false;
	}
	else {
		//original test - if is campaign
		return *GameEngine == 1;
	}
}
skull_enabled_flags* SkullFlags;
static bool __cdecl RenderFirstPersonCheck(unsigned int a1)
{
	if (AdvLobbySettings_mp_blind & 0b10)
		return true;
	return !b_showFirstPerson;
}
static bool __cdecl RenderHudCheck(unsigned int a1)
{
	// TODO: cleanup
	static bool hud_opacity_reset = true;
	DWORD new_hud_globals = *(DWORD*)(H2BaseAddr + 0x9770F4);
	float& hud_opacity = *(float*)(new_hud_globals + 0x228); // set the opacity

	if (!b_showHUD || SkullFlags->Blind)
	{
		hud_opacity = 0.f;
		hud_opacity_reset = false;
	}
	else if (!hud_opacity_reset)
	{
		hud_opacity = 1.f;
		hud_opacity_reset = true;
	}

	return false;
}

bool crosshairInit = false;
static point2d defaultCrosshairSizes[59];
void HudElements::setCrosshairSize()
{
	if (h2mod->Server)
		return;
	if (h2mod->GetMapType() == scnr_type::Multiplayer) {
		point2d* Weapons[59];

		auto hud_reticles = tags::find_tag(blam_tag::tag_group_type::bitmap, "ui\\hud\\bitmaps\\new_hud\\crosshairs\\hud_reticles");
		char* hud_reticles_data = tags::get_tag<blam_tag::tag_group_type::bitmap, char>(hud_reticles);
		tags::tag_data_block* hud_reticles_bitmaps = reinterpret_cast<tags::tag_data_block*>(hud_reticles_data + 0x44);
		if (hud_reticles_bitmaps->block_data_offset != -1)
		{
			char* reticle_bitmap = tags::get_tag_data() + hud_reticles_bitmaps->block_data_offset;
			for (auto i = 0; i < hud_reticles_bitmaps->block_count; i++)
			{
				point2d* ui_bitmap_size = reinterpret_cast<point2d*>(reticle_bitmap + (i * 0x74) + 0x4);
				Weapons[i] = ui_bitmap_size;
				if (!crosshairInit) {
					defaultCrosshairSizes[i].x = ui_bitmap_size->x;
					defaultCrosshairSizes[i].y = ui_bitmap_size->y;
				}
			}
		}
		crosshairInit = true;
		for (int i = 0; i < 59; i++) {
			*Weapons[i] = *new point2d{ (short)round(defaultCrosshairSizes[i].x * H2Config_crosshair_scale), (short)round(defaultCrosshairSizes[i].y * H2Config_crosshair_scale) };
		}
	}
}
void HudElements::setCrosshairPos() {

	if (h2mod->Server)
		return;
	commands->display("Setting Chrosshair position");
	if (!FloatIsNaN(H2Config_crosshair_offset)) {
		commands->display("Crosshair override position is:" + std::to_string(H2Config_crosshair_offset));
		tags::tag_data_block* player_controls_block = reinterpret_cast<tags::tag_data_block*>(tags::get_matg_globals_ptr() + 240);
		commands->display("Player_Controls_Block: " + IntToString<int>(player_controls_block->block_data_offset, std::hex) + " : " + IntToString<int>(player_controls_block->block_count));
		if (player_controls_block->block_count > 0)
		{
			for (int i = 0; i < player_controls_block->block_count; i++) {
				commands->display("Setting Player Control Tag Block Index: " + IntToString<int>(i));
				*(float*)(tags::get_tag_data() + player_controls_block->block_data_offset + 128 * i + 28) = H2Config_crosshair_offset;
			}
		}
	}
}


void HudElements::RadarPatch()
{
	WriteValue<BYTE>(H2BaseAddr + 0x2849C4, (BYTE)4);
	addDebugText("Motion sensor patched successfully.");
}
void HudElements::setFOV() {

	if (h2mod->Server)
		return;

	static float fov = 70.0f * M_PI / 180.0f;
	static bool fov_redirected = false;
	if (H2Config_field_of_view > 0 && H2Config_field_of_view <= 110)
	{
		if (!fov_redirected)
		{
			BYTE opcode[6] = { 0xD9, 0x05, 0x00, 0x00, 0x00, 0x00 };
			WritePointer((DWORD)&opcode[2], &fov);
			WriteBytes(h2mod->GetAddress(0x907F3), opcode, sizeof(opcode)); // fld dword ptr[fov]

			fov_redirected = true;
		}

		//const double default_radians_field_of_view = 70.0f * M_PI / 180.0f;
		fov = (float)H2Config_field_of_view * M_PI / 180.0f;
	}
}

void HudElements::setVehicleFOV() {

	if (h2mod->Server)
		return;

	if (H2Config_vehicle_field_of_view > 0 && H2Config_vehicle_field_of_view <= 110)
	{
		float calculated_radians_FOV = (float)H2Config_vehicle_field_of_view * M_PI / 180.0f;
		WriteValue(h2mod->GetAddress(0x413780), calculated_radians_FOV); // Third Person
	}
}


void HudElements::ToggleFirstPerson(bool state)
{
	if (state == NULL)
		b_showFirstPerson = !b_showFirstPerson;
	else
		b_showFirstPerson = state;
}

void HudElements::ToggleHUD(bool state)
{
	if (state == NULL)
		b_showHUD = !b_showHUD;
	else
		b_showHUD = state;
}


void HudElements::OnMapLoad()
{
	if (h2mod->GetMapType() == Multiplayer)
	{
		//Fix the Master Chief FP Arms Shader
		auto fp_shader_datum = tags::find_tag(blam_tag::tag_group_type::shader, "objects\\characters\\masterchief\\fp\\shaders\\fp_arms");
		BYTE* fp_shader_tag_data = tags::get_tag<blam_tag::tag_group_type::shader, BYTE>(fp_shader_datum);
		if (fp_shader_tag_data != nullptr)
			*(float*)(fp_shader_tag_data + 0x44) = 1;

		setCrosshairSize();
		setCrosshairPos();
	}
}
void HudElements::ApplyHooks()
{
	//Redirects the is_campaign call that the in-game chat renderer makes so we can show/hide it as we like.
	PatchCall(h2mod->GetAddress(0x22667B), RenderIngameChat);
	PatchCall(h2mod->GetAddress(0x226628), RenderIngameChat);
	PatchCall(h2mod->GetAddress(0x228579), RenderFirstPersonCheck);
	PatchCall(h2mod->GetAddress(0x223955), RenderHudCheck);
}
void HudElements::Init()
{
	if (H2IsDediServer)
		return;
	SkullFlags = reinterpret_cast<skull_enabled_flags*>(h2mod->GetAddress(0x4D8320));
	ApplyHooks();
	setFOV();
	setVehicleFOV();
	RadarPatch();
}
