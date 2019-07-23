Clock@ clock = engine.clock();
Camera@ camera = engine.camera();
Player@ player = engine.player();
Character@ dude = engine.character("dude");

void main() {
    clock.on("change", "5m", function() {
        print("Clock change");
        print("Camera " + camera.x() + " " + camera.y());
        print("Clock " + clock.year() + "-" + clock.month() + "-" + clock.day() + " " + clock.hour() + ":" + clock.minute());
        print("Dude " + dude.x() + " " + dude.y());

        player.setCanControl(false);
        camera.moveTo(290, 290, function() {
            camera.follow(dude);
            print("camera finished");

            dude.moveTo(290, 290, function() {
                camera.follow(player.character());
                player.setCanControl(true);
                print("move done");
            });
        });
    });
}