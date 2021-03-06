#include <stdlib.h>

#include "./define.h"
#include "./player.h"

UINT8 _PLAYER_FRAMES_DOWN[] = {0x00, 0x04};
UINT8 _PLAYER_FRAMES_UP[] = {0x20, 0x24};
UINT8 _PLAYER_FRAMES_RIGHT[] = {0x44, 0x40};

UINT8 _PLAYER_FRAMES_HIT_DOWN[] = {0x10, 0x14};
UINT8 _PLAYER_FRAMES_HIT_UP[] = {0x28, 0x2C};
UINT8 _PLAYER_FRAMES_HIT_RIGHT[] = {0x48, 0x50};

UINT8 _PLAYER_FRAMES_SWORD_DOWN[] = {0x30, 0x34};
UINT8 _PLAYER_FRAMES_SWORD_UP[] = {0x08, 0x0C};
UINT8 _PLAYER_FRAMES_SWORD_RIGHT[] = {0x4C, 0x54};

Player* player_new() {
    Player* player = malloc(sizeof(Player));

    if (player == NULL) {
        return NULL;
    }

    player->sprite = sprite16_new(
            SPRITE_PLAYER,
            _PLAYER_FRAMES_DOWN[0],
            GB_SCREEN_CENTER_X * GB_TILE_SIZE + GB_SPRITE_OFFSET_X,
            GB_SCREEN_CENTER_Y * GB_TILE_SIZE + GB_SPRITE_OFFSET_Y);

    player->sword = sprite16_new(
            SPRITE_SWORD,
            _PLAYER_FRAMES_SWORD_DOWN[0],
            0,
            0);

    player->anim_down = sprite16anim_new(player->sprite, 6, 2, _PLAYER_FRAMES_DOWN, FALSE);
    player->anim_up = sprite16anim_new(player->sprite, 6, 2, _PLAYER_FRAMES_UP, FALSE);
    player->anim_right = sprite16anim_new(player->sprite, 6, 2, _PLAYER_FRAMES_RIGHT, FALSE);
    player->anim_left = sprite16anim_new(player->sprite, 6, 2, _PLAYER_FRAMES_RIGHT, TRUE);

    player->anim_hit_down = sprite16anim_new(player->sprite, 6, 2, _PLAYER_FRAMES_HIT_DOWN, FALSE);
    player->anim_hit_up = sprite16anim_new(player->sprite, 6, 2, _PLAYER_FRAMES_HIT_UP, FALSE);
    player->anim_hit_right = sprite16anim_new(player->sprite, 6, 2, _PLAYER_FRAMES_HIT_RIGHT, FALSE);
    player->anim_hit_left = sprite16anim_new(player->sprite, 6, 2, _PLAYER_FRAMES_HIT_RIGHT, TRUE);

    player->anim_sword_down = sprite16anim_new(player->sword, 6, 2, _PLAYER_FRAMES_SWORD_DOWN, FALSE);
    player->anim_sword_up = sprite16anim_new(player->sword, 6, 2, _PLAYER_FRAMES_SWORD_UP, FALSE);
    player->anim_sword_right = sprite16anim_new(player->sword, 6, 2, _PLAYER_FRAMES_SWORD_RIGHT, FALSE);
    player->anim_sword_left = sprite16anim_new(player->sword, 6, 2, _PLAYER_FRAMES_SWORD_RIGHT, TRUE);

    player->screen_x = GB_SCREEN_CENTER_X;
    player->screen_y = GB_SCREEN_CENTER_Y;
    player->screen_x_px = GB_SCREEN_CENTER_X * GB_TILE_SIZE;
    player->screen_y_px = GB_SCREEN_CENTER_Y * GB_TILE_SIZE;
    player->dx = 0;
    player->dy = 1;
    player->smooth_scrolling = FALSE;

    player->_walk_to_count = 0;
    player->_hit_animation_frame_count = 0;
    player->_is_walking = FALSE;
    player->_is_hitting = FALSE;
    player->_anim_current = player->anim_down;

    return player;
}

// -1 <= dx <= 1
// -1 <= dy <= 1
void player_walk_to_cell(Player* player, Map* map, INT8 dx, INT8 dy) {
    Sprite16Anim* target_anim = NULL;

    // Player is using its sword
    if (player->_is_hitting) {
        return;
    }

    player->_is_walking = TRUE;

    // Already in movement
    if (player->_walk_to_count) {
        return;
    }

    // Avoid moving both verticaly and horizontaly
    if (dx) {
        dy = 0;
    }

    // Animate
    if (dx) {
        if (dx == 1) {
            target_anim = player->anim_right;
        } else {
            target_anim = player->anim_left;
        }
    } else if (dy) {
        if (dy == 1) {
            target_anim = player->anim_down;
        } else {
            target_anim = player->anim_up;
        }
    }

    // Update direction
    player->dx = dx;
    player->dy = dy;

    if (target_anim != NULL) {
        if (target_anim != player->_anim_current) {
            sprite16anim_stop(player->_anim_current);
        }
        player->_anim_current = target_anim;
        sprite16anim_play(target_anim);
    }

    // Check for collision
    if (!map_cell_is_walkable(
                map,
                map->x + dx * 2 + player->screen_x,
                map->y + dy * 2 + player->screen_y)) {
        return;
    }

    // Move
    player->_walk_to_count = MAP_CELL_SIZE;
}

void player_hit(Player* player) {
    if (player->_is_hitting) {
        return;
    }

    player->_is_hitting = TRUE;
    player->_hit_animation_frame_count = 3;
    player->_anim_paused = player->_anim_current;

    if (player->_anim_current == player->anim_down) {
        player->_anim_current = player->anim_hit_down;
        player->_anim_sword_current = player->anim_sword_down;
        sprite16_set_position(
                player->sword,
                GB_SPRITE_OFFSET_X + player->screen_x * GB_TILE_SIZE,
                GB_SPRITE_OFFSET_Y + (player->screen_y + 2) * GB_TILE_SIZE);
    } else if (player->_anim_current == player->anim_up) {
        player->_anim_current = player->anim_hit_up;
        player->_anim_sword_current = player->anim_sword_up;
        sprite16_set_position(
                player->sword,
                GB_SPRITE_OFFSET_X + player->screen_x * GB_TILE_SIZE,
                GB_SPRITE_OFFSET_Y + (player->screen_y - 2) * GB_TILE_SIZE);
    } else if (player->_anim_current == player->anim_right) {
        player->_anim_current = player->anim_hit_right;
        player->_anim_sword_current = player->anim_sword_right;
        sprite16_set_position(
                player->sword,
                GB_SPRITE_OFFSET_X + (player->screen_x + 2) * GB_TILE_SIZE,
                GB_SPRITE_OFFSET_Y + player->screen_y * GB_TILE_SIZE);
    } else if (player->_anim_current == player->anim_left) {
        player->_anim_current = player->anim_hit_left;
        player->_anim_sword_current = player->anim_sword_left;
        sprite16_set_position(
                player->sword,
                GB_SPRITE_OFFSET_X + (player->screen_x - 2) * GB_TILE_SIZE,
                GB_SPRITE_OFFSET_Y + player->screen_y * GB_TILE_SIZE);
    }

    sprite16anim_pause(player->_anim_paused);
    sprite16anim_play(player->_anim_current);
    sprite16anim_play(player->_anim_sword_current);
}

void player_hit_end(Player* player) {
    player->_is_hitting = FALSE;
    sprite16_hide(player->sword);
    sprite16anim_stop(player->_anim_current);
    sprite16anim_stop(player->_anim_sword_current);
    player->_anim_current = player->_anim_paused;
    player->_anim_paused = NULL;
    sprite16anim_play(player->_anim_current);
}

void player_update(Player* player, Map* map) {
    UINT8 i;
    // Sword action
    if (player->_is_hitting) {
        if (player->_hit_animation_frame_count) {
            if (player->_anim_current->_frame_counter == 0) {
                player->_hit_animation_frame_count -= 1;
            }
        } else {
            player_hit_end(player);
        }
    } else if (player->smooth_scrolling) {
        // Update animation
        if (!player->_is_walking) {
            sprite16anim_stop(player->_anim_current);
        }
        // Move player
        if (player->_walk_to_count) {
            player->_walk_to_count -= 1;
            map_scroll(map, player->dx, player->dy);
        }
        if (!player->_walk_to_count) {
            player->_is_walking = FALSE;
        }
    } else {
        // Update animation
        if (!player->_is_walking) {
            sprite16anim_stop(player->_anim_current);
        }
        // Move player
        if (player->_walk_to_count) {
            player->_walk_to_count -= 1;
            player->screen_x_px += player->dx;
            player->screen_y_px += player->dy;
            sprite16_set_position(
                    player->sprite,
                    GB_SPRITE_OFFSET_X + player->screen_x_px,
                    GB_SPRITE_OFFSET_Y + player->screen_y_px);
            if (
                    player->screen_y == 2 && player->dy == -1 ||
                    player->screen_y == 12 && player->dy == 1 ||
                    player->screen_x == 0 && player->dx == -1 ||
                    player->screen_x == 16 && player->dx == 1
                    ) {
                for (i = (player->dy) ? 40 : 64 ; i ; i--) {
                    map_scroll(map, player->dx, player->dy);
                    map_scroll(map, player->dx, player->dy);
                    player->screen_x_px += -2 * player->dx;
                    player->screen_y_px += -2 * player->dy;
                    sprite16_set_position(
                            player->sprite,
                            GB_SPRITE_OFFSET_X + player->screen_x_px,
                            GB_SPRITE_OFFSET_Y + player->screen_y_px);
                    wait_vbl_done();
                }
            }
            player->screen_x = player->screen_x_px >> 3;
            player->screen_y = player->screen_y_px >> 3;
        }
        if (!player->_walk_to_count) {
            player->_is_walking = FALSE;
        }
    }
}

void player_free(Player* player) {
    // TODO
}
