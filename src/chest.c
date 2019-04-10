#include <gb/gb.h>

#include "./define.h"
#include "./game.h"
#include "./text.h"
#include "./objects/chests.objects.h"


UINT8 chest_get_id(UINT8 x, UINT8 y) {
    UINT8 i;
    for (i = 0 ; i < CHESTS_COUNT ; i += 1) {
        if (CHESTS[i * 3] != x) {
            continue;
        }
        if (CHESTS[i * 3 + 1] != y) {
            continue;
        }
        return CHESTS[i * 3 + 2];
    }
    return 255;
}

void chest_activate(GameState* game_state, UINT8 chest_id) {
    switch(chest_id) {
        case 0:
            text_show_message("D-PAD Left\0", 4);  // FIXME
            game_state->dpad_mask |= J_LEFT;
            break;
        case 1:
            text_show_message("2D moves\0", 4);  // FIXME
            game_state->dpad_mask |= J_UP | J_DOWN;
            break;
        case 5:
            text_show_message("Sword\0", 4);  // FIXME
            game_state->player_has_sword = TRUE;
            break;
        default:
            text_show_message("...\0", 4);  // FIXME
    }
}