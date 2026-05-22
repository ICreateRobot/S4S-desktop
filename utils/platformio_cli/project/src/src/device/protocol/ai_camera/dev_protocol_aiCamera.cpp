/*
 * @Author       : 蔡雅超 (ZIShen)
 * @LastEditors  : zishen
 * @Date         : 2025-11-29 19:39:25
 * @LastEditTime : 2026-05-21 19:47:12
 * @Description  : k210 设备
 * Copyright (c) 2025 Author 蔡雅超 email: 2672632650@qq.com, All Rights Reserved.
 */
#include "dev_protocol_aiCamera.h"
#include "hardware/ai_camera/hw_aiCamera.h"


/******************
 * data struct
 *****************/
#define LOG_TAG "dev_protocol_aiCamera"


/****************************
 * function declaration
 ***************************/
static fmap_result_t fMap_set_i2c_port(udc_pack_t *pack);
static fmap_result_t fMap_set_mode(udc_pack_t *pack);
static fmap_result_t fMap_get_mode(udc_pack_t *pack);
static fmap_result_t fMap_color_value(udc_pack_t *pack);
static fmap_result_t fMap_set_color(udc_pack_t *pack);
static fmap_result_t fMap_color_detected(udc_pack_t *pack);
static fmap_result_t fMap_color_position(udc_pack_t *pack);
static fmap_result_t fMap_tag_count(udc_pack_t *pack);
static fmap_result_t fMap_tag_id(udc_pack_t *pack);
static fmap_result_t fMap_tag_rotation(udc_pack_t *pack);
static fmap_result_t fMap_tag_position(udc_pack_t *pack);
static fmap_result_t fMap_line_detected(udc_pack_t *pack);
static fmap_result_t fMap_line_position(udc_pack_t *pack);
static fmap_result_t fMap_object_count(udc_pack_t *pack);
static fmap_result_t fMap_object_detected(udc_pack_t *pack);
static fmap_result_t fMap_object_position(udc_pack_t *pack);
static fmap_result_t fMap_qr_detected(udc_pack_t *pack);
static fmap_result_t fMap_qr_data(udc_pack_t *pack);
static fmap_result_t fMap_qr_position(udc_pack_t *pack);
static fmap_result_t fMap_face_count(udc_pack_t *pack);
static fmap_result_t fMap_face_position(udc_pack_t *pack);
static fmap_result_t fMap_face_attribute(udc_pack_t *pack);
static fmap_result_t fMap_face_recognized_learn(udc_pack_t *pack);
static fmap_result_t fMap_face_recognized_count(udc_pack_t *pack);
static fmap_result_t fMap_face_recognized_detected(udc_pack_t *pack);
static fmap_result_t fMap_face_recognized_position(udc_pack_t *pack);
static fmap_result_t fMap_class_recognized(udc_pack_t *pack);
static fmap_result_t fMap_card_count(udc_pack_t *pack);
static fmap_result_t fMap_card_detected(udc_pack_t *pack);
static fmap_result_t fMap_card_position(udc_pack_t *pack);
static fmap_result_t fMap_state_is(udc_pack_t *pack);
static fmap_result_t fMap_motion_command_detected(udc_pack_t *pack);
static fmap_result_t fMap_motion_speed(udc_pack_t *pack);
static fmap_result_t fMap_custom_command(udc_pack_t *pack);
static fmap_result_t fMap_joystick_position(udc_pack_t *pack);
static fmap_result_t fMap_button_pressed(udc_pack_t *pack);
static fmap_result_t fMap_key_pressed(udc_pack_t *pack);
static fmap_result_t fMap_fill_light(udc_pack_t *pack);
static fmap_result_t fMap_set_fill_light_brightness(udc_pack_t *pack);
static fmap_result_t fMap_get_fill_light_brightness(udc_pack_t *pack);


/********************
 * static variables
 *******************/
static const function_map_t function_map = {
    // clang-format off
    {"set_i2c_port",                 fMap_set_i2c_port},
    {"set_mode",                     fMap_set_mode},
    {"get_mode",                     fMap_get_mode},
    {"color_value",                  fMap_color_value},
    {"set_color",                    fMap_set_color},
    {"color_detected",               fMap_color_detected},
    {"color_position",               fMap_color_position},
    {"tag_count",                    fMap_tag_count},
    {"tag_id",                       fMap_tag_id},
    {"tag_rotation",                 fMap_tag_rotation},
    {"tag_position",                 fMap_tag_position},
    {"line_detected",                fMap_line_detected},
    {"line_position",                fMap_line_position},
    {"object_count",                 fMap_object_count},
    {"object_detected",              fMap_object_detected},
    {"object_position",              fMap_object_position},
    {"qr_detected",                  fMap_qr_detected},
    {"qr_data",                      fMap_qr_data},
    {"qr_position",                  fMap_qr_position},
    {"face_count",                   fMap_face_count},
    {"face_position",                fMap_face_position},
    {"face_attribute",               fMap_face_attribute},
    {"face_recognized_learn",        fMap_face_recognized_learn},
    {"face_recognized_count",        fMap_face_recognized_count},
    {"face_recognized_detected",     fMap_face_recognized_detected},
    {"face_recognized_position",     fMap_face_recognized_position},
    {"class_recognized",             fMap_class_recognized},
    {"card_count",                   fMap_card_count},
    {"card_detected",                fMap_card_detected},
    {"card_position",                fMap_card_position},
    {"state_is",                     fMap_state_is},
    {"motion_command_detected",      fMap_motion_command_detected},
    {"motion_speed",                 fMap_motion_speed},
    {"custom_command",               fMap_custom_command},
    {"joystick_position",            fMap_joystick_position},
    {"button_pressed",               fMap_button_pressed},
    {"key_pressed",                  fMap_key_pressed},
    {"fill_light",                   fMap_fill_light},
    {"set_fill_light_brightness",    fMap_set_fill_light_brightness},
    {"get_fill_light_brightness",    fMap_get_fill_light_brightness},
    // clang-format on
};


static const enum_map_t enum_map = {
    // clang-format off
    {"COLOR",           (int)hw_ai_camera.COLOR},
    {"COLOR_TRACK",     (int)hw_ai_camera.COLOR_TRACK},
    {"TAG",             (int)hw_ai_camera.TAG},
    {"LINE",            (int)hw_ai_camera.LINE},
    {"OBJECT",          (int)hw_ai_camera.OBJECT},
    {"QR",              (int)hw_ai_camera.QR},
    {"FACE_DETECT",     (int)hw_ai_camera.FACE_DETECT},
    {"FACE_RECOGNIZE",  (int)hw_ai_camera.FACE_RECOGNIZE},
    {"AI",              (int)hw_ai_camera.AI},
    {"CARD",            (int)hw_ai_camera.CARD},

    {"AXIS_X",          (int)hw_ai_camera.AXIS_X},
    {"AXIS_Y",          (int)hw_ai_camera.AXIS_Y},
    {"AXIS_W",          (int)hw_ai_camera.AXIS_W},
    {"AXIS_H",          (int)hw_ai_camera.AXIS_H},
    {"UP",              (int)hw_ai_camera.UP},
    {"MIDDLE",          (int)hw_ai_camera.MIDDLE},
    {"DOWN",            (int)hw_ai_camera.DOWN},

    {"GO_FORWARD",      (int)hw_ai_camera.GO_FORWARD},
    {"GO_BACKWARD",     (int)hw_ai_camera.GO_BACKWARD},
    {"TURN_LEFT",       (int)hw_ai_camera.TURN_LEFT},
    {"TURN_RIGHT",      (int)hw_ai_camera.TURN_RIGHT},
    {"STOP_MOVING",     (int)hw_ai_camera.STOP_MOVING},

    {"AI_OFF",          (int)hw_ai_camera.AI_OFF},
    {"AI_CONNECTING",   (int)hw_ai_camera.AI_CONNECTING},
    {"AI_IDLE",         (int)hw_ai_camera.AI_IDLE},
    {"AI_LISTENING",    (int)hw_ai_camera.AI_LISTENING},
    {"AI_SPEAKING",     (int)hw_ai_camera.AI_SPEAKING},
    {"AI_CONFIGURING",  (int)hw_ai_camera.AI_CONFIGURING},

    {"RED",             (int)hw_ai_camera.RED},
    {"GREEN",           (int)hw_ai_camera.GREEN},
    {"BLUE",            (int)hw_ai_camera.BLUE},
    {"YELLOW",          (int)hw_ai_camera.YELLOW},
    {"BLACK",           (int)hw_ai_camera.BLACK},
    {"WHITE",           (int)hw_ai_camera.WHITE},

    {"OBJECT_AIRPLANE",     (int)hw_ai_camera.OBJECT_AIRPLANE},
    {"OBJECT_BICYCLE",      (int)hw_ai_camera.OBJECT_BICYCLE},
    {"OBJECT_BIRD",         (int)hw_ai_camera.OBJECT_BIRD},
    {"OBJECT_BOAT",         (int)hw_ai_camera.OBJECT_BOAT},
    {"OBJECT_BOTTLE",       (int)hw_ai_camera.OBJECT_BOTTLE},
    {"OBJECT_BUS",          (int)hw_ai_camera.OBJECT_BUS},
    {"OBJECT_CAR",          (int)hw_ai_camera.OBJECT_CAR},
    {"OBJECT_CAT",          (int)hw_ai_camera.OBJECT_CAT},
    {"OBJECT_CHAIR",        (int)hw_ai_camera.OBJECT_CHAIR},
    {"OBJECT_COW",          (int)hw_ai_camera.OBJECT_COW},
    {"OBJECT_DININGTABLE",  (int)hw_ai_camera.OBJECT_DININGTABLE},
    {"OBJECT_DOG",          (int)hw_ai_camera.OBJECT_DOG},
    {"OBJECT_HORSE",        (int)hw_ai_camera.OBJECT_HORSE},
    {"OBJECT_MOTORBIKE",    (int)hw_ai_camera.OBJECT_MOTORBIKE},
    {"OBJECT_PERSON",       (int)hw_ai_camera.OBJECT_PERSON},
    {"OBJECT_POTTEDPLANT",  (int)hw_ai_camera.OBJECT_POTTEDPLANT},
    {"OBJECT_SHEEP",        (int)hw_ai_camera.OBJECT_SHEEP},
    {"OBJECT_SOFA",         (int)hw_ai_camera.OBJECT_SOFA},
    {"OBJECT_TRAIN",        (int)hw_ai_camera.OBJECT_TRAIN},
    {"OBJECT_TV",           (int)hw_ai_camera.OBJECT_TV},

    {"EVENT_HONK",          (int)hw_ai_camera.EVENT_HONK},
    {"EVENT_TARGET",        (int)hw_ai_camera.EVENT_TARGET},

    {"FACE_OPEN_MOUTH",     (int)hw_ai_camera.FACE_OPEN_MOUTH},
    {"FACE_SMILE",          (int)hw_ai_camera.FACE_SMILE},
    {"FACE_GLASSES",        (int)hw_ai_camera.FACE_GLASSES},
    // clang-format on
};
/********************
 * global variables
 *******************/


/********************
 * global functions
 *******************/
void dev_protocol_aiCamera_init(void)
{
    function_map_collection["vision"] = &function_map;
}


/****************************
 * static function
 ***************************/
static fmap_result_t fMap_set_i2c_port(udc_pack_t *pack)
{
    int ret = 0;
    int port;
    function_map_udcpack_get_param_int(pack, function_map_udcpack_id(0), &port);
    ret = hw_ai_camera.set_i2c_port(port);
    return fmap_result_t::make_result(ret);
}

static fmap_result_t fMap_set_mode(udc_pack_t *pack)
{
    int         ret = 0;
    std::string mode;
    int         mode_int;

    function_map_udcpack_get_param_string(pack, function_map_udcpack_id(0), &mode);
    enum_map_get_value(enum_map, mode.c_str(), mode_int);

    ret = hw_ai_camera.set_mode(mode_int);

    return fmap_result_t::make_result(ret);
}

static fmap_result_t fMap_get_mode(udc_pack_t *pack)
{
    int ret = 0;
    ret     = hw_ai_camera.get_mode();

    if (ret < 0)
        return fmap_result_t::make_error(ret);
    else
        return fmap_result_t::make_ok().add_int(ret);
}

static fmap_result_t fMap_color_value(udc_pack_t *pack)
{
    int         ret = 0;
    std::string color;
    int         color_int;

    function_map_udcpack_get_param_string(pack, function_map_udcpack_id(0), &color);
    enum_map_get_value(enum_map, color.c_str(), color_int);

    ret = hw_ai_camera.color_value(color_int);

    if (ret < 0)
        return fmap_result_t::make_error(ret);
    else
        return fmap_result_t::make_ok().add_int(ret);
}

static fmap_result_t fMap_set_color(udc_pack_t *pack)
{
    int         ret = 0;
    std::string color;
    int         color_int;

    function_map_udcpack_get_param_string(pack, function_map_udcpack_id(0), &color);
    enum_map_get_value(enum_map, color.c_str(), color_int);

    ret = hw_ai_camera.set_color(color_int);

    return fmap_result_t::make_result(ret);
}

static fmap_result_t fMap_color_detected(udc_pack_t *pack)
{
    int ret = 0;
    ret     = hw_ai_camera.color_detected();

    if (ret < 0)
        return fmap_result_t::make_error(ret);
    else
        return fmap_result_t::make_ok().add_int(ret);
}

static fmap_result_t fMap_color_position(udc_pack_t *pack)
{
    int         ret = 0;
    std::string select;
    int         select_int;

    function_map_udcpack_get_param_string(pack, function_map_udcpack_id(0), &select);
    enum_map_get_value(enum_map, select.c_str(), select_int);

    ret = hw_ai_camera.color_position(select_int);

    if (ret < 0)
        return fmap_result_t::make_error(ret);
    else
        return fmap_result_t::make_ok().add_int(ret);
}

static fmap_result_t fMap_tag_count(udc_pack_t *pack)
{
    int ret = 0;
    ret     = hw_ai_camera.tag_count();

    if (ret < 0)
        return fmap_result_t::make_error(ret);
    else
        return fmap_result_t::make_ok().add_int(ret);
}

static fmap_result_t fMap_tag_id(udc_pack_t *pack)
{
    int ret = 0;
    int id  = 1;
    function_map_udcpack_get_param_int_default(pack, function_map_udcpack_id(0), &id);
    ret = hw_ai_camera.tag_id(id);

    if (ret < 0)
        return fmap_result_t::make_error(ret);
    else
        return fmap_result_t::make_ok().add_int(ret);
}

static fmap_result_t fMap_tag_rotation(udc_pack_t *pack)
{
    int ret = 0;
    int id  = 1;
    function_map_udcpack_get_param_int_default(pack, function_map_udcpack_id(0), &id);
    ret = hw_ai_camera.tag_rotation(id);

    if (ret < 0)
        return fmap_result_t::make_error(ret);
    else
        return fmap_result_t::make_ok().add_int(ret);
}

static fmap_result_t fMap_tag_position(udc_pack_t *pack)
{
    int         ret = 0;
    std::string select;
    int         select_int;
    int         id = 1;

    function_map_udcpack_get_param_string(pack, function_map_udcpack_id(0), &select);
    enum_map_get_value(enum_map, select.c_str(), select_int);
    function_map_udcpack_get_param_int_default(pack, function_map_udcpack_id(1), &id);

    ret = hw_ai_camera.tag_position(select_int, id);

    return fmap_result_t::make_ok().add_int(ret);
}

static fmap_result_t fMap_line_detected(udc_pack_t *pack)
{
    int ret = 0;
    ret     = hw_ai_camera.line_detected();

    if (ret < 0)
        return fmap_result_t::make_error(ret);
    else
        return fmap_result_t::make_ok().add_int(ret);
}

static fmap_result_t fMap_line_position(udc_pack_t *pack)
{
    int         ret = 0;
    std::string region, info;
    int         region_int, info_int;

    function_map_udcpack_get_param_string(pack, function_map_udcpack_id(0), &region);
    enum_map_get_value(enum_map, region.c_str(), region_int);
    function_map_udcpack_get_param_string(pack, function_map_udcpack_id(1), &info);
    enum_map_get_value(enum_map, info.c_str(), info_int);

    ret = hw_ai_camera.line_position(region_int, info_int);

    return fmap_result_t::make_ok().add_int(ret);
}

static fmap_result_t fMap_object_count(udc_pack_t *pack)
{
    int ret = 0;
    ret     = hw_ai_camera.object_count();

    if (ret < 0)
        return fmap_result_t::make_error(ret);
    else
        return fmap_result_t::make_ok().add_int(ret);
}

static fmap_result_t fMap_object_detected(udc_pack_t *pack)
{
    int         ret = 0;
    std::string object_class;
    int         object_class_int;
    int         id = 1;

    function_map_udcpack_get_param_string(pack, function_map_udcpack_id(0), &object_class);
    enum_map_get_value(enum_map, object_class.c_str(), object_class_int);
    function_map_udcpack_get_param_int_default(pack, function_map_udcpack_id(1), &id);

    ret = hw_ai_camera.object_detected(object_class_int, id);

    if (ret < 0)
        return fmap_result_t::make_error(ret);
    else
        return fmap_result_t::make_ok().add_int(ret);
}

static fmap_result_t fMap_object_position(udc_pack_t *pack)
{
    int         ret = 0;
    std::string select;
    int         select_int;
    int         id = 1;

    function_map_udcpack_get_param_string(pack, function_map_udcpack_id(0), &select);
    enum_map_get_value(enum_map, select.c_str(), select_int);
    function_map_udcpack_get_param_int_default(pack, function_map_udcpack_id(1), &id);

    ret = hw_ai_camera.object_position(select_int, id);

    return fmap_result_t::make_ok().add_int(ret);
}

static fmap_result_t fMap_qr_detected(udc_pack_t *pack)
{
    int ret = 0;
    ret     = hw_ai_camera.qr_detected();

    if (ret < 0)
        return fmap_result_t::make_error(ret);
    else
        return fmap_result_t::make_ok().add_int(ret);
}

static fmap_result_t fMap_qr_data(udc_pack_t *pack)
{
    String ret = hw_ai_camera.qr_data();

    if (ret.length() == 0)
        return fmap_result_t::make_error(-1);
    else
        return fmap_result_t::make_ok().add_string(ret.c_str());
}

static fmap_result_t fMap_qr_position(udc_pack_t *pack)
{
    int         ret = 0;
    std::string select;
    int         select_int;

    function_map_udcpack_get_param_string(pack, function_map_udcpack_id(0), &select);
    enum_map_get_value(enum_map, select.c_str(), select_int);

    ret = hw_ai_camera.qr_position(select_int);


    return fmap_result_t::make_ok().add_int(ret);
}

static fmap_result_t fMap_face_count(udc_pack_t *pack)
{
    int ret = 0;
    ret     = hw_ai_camera.face_count();

    if (ret < 0)
        return fmap_result_t::make_error(ret);
    else
        return fmap_result_t::make_ok().add_int(ret);
}

static fmap_result_t fMap_face_position(udc_pack_t *pack)
{
    int         ret = 0;
    std::string select;
    int         select_int;
    int         id = 1;

    function_map_udcpack_get_param_string(pack, function_map_udcpack_id(0), &select);
    enum_map_get_value(enum_map, select.c_str(), select_int);
    function_map_udcpack_get_param_int_default(pack, function_map_udcpack_id(1), &id);

    ret = hw_ai_camera.face_position(select_int, id);

    return fmap_result_t::make_ok().add_int(ret);
}

static fmap_result_t fMap_face_attribute(udc_pack_t *pack)
{
    int         ret = 0;
    std::string attribute;
    int         attribute_int;
    int         id = 1;

    function_map_udcpack_get_param_string(pack, function_map_udcpack_id(0), &attribute);
    enum_map_get_value(enum_map, attribute.c_str(), attribute_int);
    function_map_udcpack_get_param_int_default(pack, function_map_udcpack_id(1), &id);

    ret = hw_ai_camera.face_attribute(attribute_int, id);

    if (ret < 0)
        return fmap_result_t::make_error(ret);
    else
        return fmap_result_t::make_ok().add_int(ret);
}

static fmap_result_t fMap_face_recognized_learn(udc_pack_t *pack)
{
    int ret = 0;
    ret     = hw_ai_camera.face_recognized_learn();

    if (ret < 0)
        return fmap_result_t::make_error(ret);
    else
        return fmap_result_t::make_ok();
}

static fmap_result_t fMap_face_recognized_count(udc_pack_t *pack)
{
    int ret = 0;
    ret     = hw_ai_camera.face_recognized_count();

    if (ret < 0)
        return fmap_result_t::make_error(ret);
    else
        return fmap_result_t::make_ok().add_int(ret);
}

static fmap_result_t fMap_face_recognized_detected(udc_pack_t *pack)
{
    int ret = 0;

    ret = hw_ai_camera.face_recognized_detected();

    if (ret < 0)
        return fmap_result_t::make_error(ret);
    else
        return fmap_result_t::make_ok().add_int(ret);
}

static fmap_result_t fMap_face_recognized_position(udc_pack_t *pack)
{
    int         ret = 0;
    std::string select;
    int         select_int;
    int         id = 1;

    function_map_udcpack_get_param_string(pack, function_map_udcpack_id(0), &select);
    enum_map_get_value(enum_map, select.c_str(), select_int);
    function_map_udcpack_get_param_int_default(pack, function_map_udcpack_id(1), &id);

    ret = hw_ai_camera.face_recognized_position(select_int, id);

    return fmap_result_t::make_ok().add_int(ret);
}

static fmap_result_t fMap_class_recognized(udc_pack_t *pack)
{
    int ret = 0;
    int class_id;

    function_map_udcpack_get_param_int_default(pack, function_map_udcpack_id(0), &class_id);

    ret = hw_ai_camera.class_recognized(class_id);

    if (ret < 0)
        return fmap_result_t::make_error(ret);
    else
        return fmap_result_t::make_ok().add_int(ret);
}

static fmap_result_t fMap_card_count(udc_pack_t *pack)
{
    int ret = 0;
    ret     = hw_ai_camera.card_count();

    if (ret < 0)
        return fmap_result_t::make_error(ret);
    else
        return fmap_result_t::make_ok().add_int(ret);
}

static fmap_result_t fMap_card_detected(udc_pack_t *pack)
{
    int         ret = 0;
    std::string type;
    int         type_int;
    int         sel, id = 1;

    function_map_udcpack_get_param_string(pack, function_map_udcpack_id(0), &type);
    enum_map_get_value(enum_map, type.c_str(), type_int);
    function_map_udcpack_get_param_int(pack, function_map_udcpack_id(1), &sel);
    function_map_udcpack_get_param_int_default(pack, function_map_udcpack_id(2), &id);

    ret = hw_ai_camera.card_detected(type_int, sel, id);

    if (ret < 0)
        return fmap_result_t::make_error(ret);
    else
        return fmap_result_t::make_ok().add_int(ret);
}

static fmap_result_t fMap_card_position(udc_pack_t *pack)
{
    int         ret = 0;
    std::string select;
    int         select_int;
    int         id = 1;

    function_map_udcpack_get_param_string(pack, function_map_udcpack_id(0), &select);
    enum_map_get_value(enum_map, select.c_str(), select_int);
    function_map_udcpack_get_param_int_default(pack, function_map_udcpack_id(1), &id);

    ret = hw_ai_camera.card_position(select_int, id);

    return fmap_result_t::make_ok().add_int(ret);
}

static fmap_result_t fMap_state_is(udc_pack_t *pack)
{
    int         ret = 0;
    std::string state;
    int         state_int;

    function_map_udcpack_get_param_string(pack, function_map_udcpack_id(0), &state);
    enum_map_get_value(enum_map, state.c_str(), state_int);

    ret = hw_ai_camera.state_is(state_int);

    if (ret < 0)
        return fmap_result_t::make_error(ret);
    else
        return fmap_result_t::make_ok().add_int(ret);
}

static fmap_result_t fMap_motion_command_detected(udc_pack_t *pack)
{
    int         ret = 0;
    std::string command;
    int         command_int;

    function_map_udcpack_get_param_string(pack, function_map_udcpack_id(0), &command);
    enum_map_get_value(enum_map, command.c_str(), command_int);

    ret = hw_ai_camera.motion_command_detected(command_int);

    if (ret < 0)
        return fmap_result_t::make_error(ret);
    else
        return fmap_result_t::make_ok().add_int(ret);
}

static fmap_result_t fMap_motion_speed(udc_pack_t *pack)
{
    int ret = 0;

    ret = hw_ai_camera.motion_speed();

    return fmap_result_t::make_ok().add_int(ret);
}

static fmap_result_t fMap_custom_command(udc_pack_t *pack)
{
    int ret = 0;

    ret = hw_ai_camera.custom_command();

    if (ret < 0)
        return fmap_result_t::make_error(ret);
    else
        return fmap_result_t::make_ok().add_int(ret);
}

static fmap_result_t fMap_joystick_position(udc_pack_t *pack)
{
    int         ret = 0;
    std::string axis;
    int         axis_int;

    function_map_udcpack_get_param_string(pack, function_map_udcpack_id(0), &axis);
    enum_map_get_value(enum_map, axis.c_str(), axis_int);

    ret = hw_ai_camera.joystick_position(axis_int);

    if (ret < 0)
        return fmap_result_t::make_error(ret);
    else
        return fmap_result_t::make_ok().add_int(ret);
}

static fmap_result_t fMap_button_pressed(udc_pack_t *pack)
{
    int ret = 0;
    int button;

    function_map_udcpack_get_param_int(pack, function_map_udcpack_id(0), &button);

    ret = hw_ai_camera.button_pressed(button);

    if (ret < 0)
        return fmap_result_t::make_error(ret);
    else
        return fmap_result_t::make_ok().add_int(ret);
}

static fmap_result_t fMap_key_pressed(udc_pack_t *pack)
{
    int ret = 0;
    int key;

    function_map_udcpack_get_param_int(pack, function_map_udcpack_id(0), &key);

    ret = hw_ai_camera.key_pressed(key);

    if (ret < 0)
        return fmap_result_t::make_error(ret);
    else
        return fmap_result_t::make_ok().add_int(ret);
}

static fmap_result_t fMap_fill_light(udc_pack_t *pack)
{
    int ret = 0;
    int light;

    function_map_udcpack_get_param_int(pack, function_map_udcpack_id(0), &light);

    ret = hw_ai_camera.fill_light(light);

    return fmap_result_t::make_result(ret);
}

static fmap_result_t fMap_set_fill_light_brightness(udc_pack_t *pack)
{
    int ret = 0;
    int brightness;

    function_map_udcpack_get_param_int(pack, function_map_udcpack_id(0), &brightness);

    ret = hw_ai_camera.set_fill_light_brightness(brightness);

    return fmap_result_t::make_result(ret);
}

static fmap_result_t fMap_get_fill_light_brightness(udc_pack_t *pack)
{
    int ret = 0;

    ret = hw_ai_camera.get_fill_light_brightness();

    if (ret < 0)
        return fmap_result_t::make_error(ret);
    else
        return fmap_result_t::make_ok().add_int(ret);
}
