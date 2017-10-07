#include <pebble.h>
static Window *s_main_window;
static TextLayer *s_time_layer;
static TextLayer *s_date_layer;
static TextLayer *s_weather_layer;
static GFont s_time_font;
static GFont s_weather_font;
static BitmapLayer *s_background_layer;
static GBitmap *s_background_bitmap;

static void update_time() {
  // Khai bao truy cap vao stuct tm
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);

  // Ghi gio phut vao mot bo dem
  static char s_buffer[8];
  strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ? "%H:%M" : "%I:%M", tick_time);
  
  // Hien thi thoi gian tren TextLayer
  text_layer_set_text(s_time_layer, s_buffer); 
}

static void update_date() {
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  static char s_date_buffer[16];
  
  strftime(s_date_buffer, sizeof(s_date_buffer), "%a, %d %b", tick_time);
  text_layer_set_text(s_date_layer, s_date_buffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
  if(units_changed == DAY_UNIT) {
    update_date();
  }
}

static void main_window_load(Window *window) {
  // Lay thong tin ve Window dang hien thi
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
  // Tao GBitmap
  s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND);
  
  // Tao BitmapLayer de hien thi GBitmap
  s_background_layer = bitmap_layer_create(bounds);
  
  // Dat anh bitmap len layer va dat len window
  bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_background_layer));
  
  // Tao TextLayer voi kich thuoc khung cu the rieng tung loai mat dong ho (pebble time/time round)
  s_time_layer = text_layer_create(GRect(0, PBL_IF_ROUND_ELSE(58, 52), bounds.size.w, 50));
  s_date_layer = text_layer_create(GRect(0, PBL_IF_ROUND_ELSE(38, 32), bounds.size.w, 30));
  
  // Tao layer hien thi nhiet do
  s_weather_layer = text_layer_create(GRect(0, PBL_IF_ROUND_ELSE(125, 120), bounds.size.w, 25));
  
  // Chinh sua layout cho giong mat dong ho hon
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorBlack);
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_text_color(s_date_layer, GColorWhite);
  
  //text_layer_set_text(s_time_layer, "00:00");
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_PERFECT_DOS_VGA_48));
  text_layer_set_font(s_time_layer, s_time_font);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  
  text_layer_set_font(s_date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
  
  // TextLayer thoi tiet
  s_weather_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_PERFECT_DOS_VGA_20));
  text_layer_set_font(s_weather_layer, s_weather_font);
  text_layer_set_background_color(s_weather_layer, GColorClear);
  text_layer_set_text_color(s_weather_layer, GColorWhite);
  text_layer_set_text_alignment(s_weather_layer, GTextAlignmentCenter);
  text_layer_set_text(s_weather_layer, "Loading...");
  
  
  // Them text layer vao Window goc
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_date_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_weather_layer));
  
  update_time();
  update_date();
}

static void main_window_unload(Window *window) {
  // Huy TextLayer
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_date_layer);
  text_layer_destroy(s_weather_layer);
  // Huy GFont
  fonts_unload_custom_font(s_time_font);
  fonts_unload_custom_font(s_weather_font);
  //Huy Gbitmap
  gbitmap_destroy(s_background_bitmap);
  // Huy BitmapLayer
  bitmap_layer_destroy(s_background_layer);
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

static void init() {
  // Khoi tao doi tuong Window va gan vao con tro
  s_main_window = window_create();
  window_set_background_color(s_main_window, GColorBlack);
  
  // Cai dat ham xu ly de quan ly cac doi tuong ben trong Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  
  // Hien thi Window (cua so) len mat dong ho, voi animated=true
  window_stack_push(s_main_window, true);
  
  // Dang ky su dung dich vu TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  
  // Dang ky cac ham goi lai cua API Message
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
  
  // Mo AppMessage
  const int inbox_size = 128;
  const int outbox_size = 128;
  app_message_open(inbox_size, outbox_size);
}

static void deinit() {
  //Huy doi tuong Window
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}