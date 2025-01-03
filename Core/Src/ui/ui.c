#include <lvgl.h>
void ui_init(){
/*Create a Tab view object*/
    lv_obj_t * tabview;
    tabview = lv_tabview_create(lv_screen_active());

    /*Add 3 tabs (the tabs are page (lv_page) and can be scrolled*/
    lv_obj_t * tab1 = lv_tabview_add_tab(tabview, "CSP");
    lv_obj_t * tab2 = lv_tabview_add_tab(tabview, "CSV");
    lv_obj_t * tab3 = lv_tabview_add_tab(tabview, "Info");

    /*Add content to the tabs*/
    lv_obj_t * label = lv_label_create(tab1);
    /* Style first tab */
    lv_obj_set_pos(label, 10, 0);
    lv_label_set_text_fmt(label, "%s%s","SafeOp","+E");

    lv_obj_t * label_tp = lv_label_create(tab1);
    lv_obj_set_pos(label_tp, 10, 40);
    lv_label_set_text_fmt(label_tp, "Target Position %i",100);

    lv_obj_t * label_ap = lv_label_create(tab1);
    lv_obj_set_pos(label_ap, 10, 80);
    lv_label_set_text_fmt(label_ap, "Actual Position %i",80);

    lv_obj_t * label_se = lv_label_create(tab1);
    lv_obj_set_pos(label_se, 0, 140);
    lv_label_set_text_fmt(label_se, "E%i",80);


    label = lv_label_create(tab2);
    /* Style second tab */
    lv_label_set_text(label, "Second tab");

    label = lv_label_create(tab3);
    /* Style third tab */
    lv_label_set_text(label, "Third tab");

//    lv_obj_scroll_to_view_recursive(label, LV_ANIM_ON);
}
