Clock@ clock = engine.clock();
Camera@ camera = engine.camera();
Player@ player = engine.player();

void main() {
    clock.on("change", "15m", function() {
        print("Clock change");
        print("Camera " + camera.x() + " " + camera.y());
        print("Clock " + clock.year() + "-" + clock.month() + "-" + clock.day() + " " + clock.hour() + ":" + clock.minute());
        print("Player " + player.character().x() + " " + player.character().y());

        player.setCanControl(false);
        camera.moveTo(290, 290, function() {
            camera.follow(player.character());
            print("camera finished");

            player.character().moveTo(290, 290, function() {
                player.setCanControl(true);
                print("move done");
            });
        });
    });
}