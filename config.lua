width = 640
height = 440
chinese_font = { path = "/Users/jjgod/Library/Fonts/方正楷体_GBK.TTF", size = 30 }
english_font = { path = "/Users/jjgod/Library/Fonts/MyriadPro-Regular.otf", size = 30 }

function init_ui()
    ui.set_title("All Heros in Kam Yung's Stories")

    ui.show_image("resource/1-1.bmp", 0, 60, 3);

    ui.draw_text("重新开始", 250, 270, 0xFF0000);
    ui.draw_text("载入进度", 250, 310, 0x990000);
    ui.draw_text("离开游戏", 250, 350, 0x990000);
end

