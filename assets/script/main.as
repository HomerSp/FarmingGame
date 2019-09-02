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

        player.setCanControl(true);
        camera.follow(dude, function() {
            print("camera finished");

            dude.moveTo(528, 0, function() {
                dude.turnTo("down");
                print("move done");
                camera.follow(player.character(), function() {
                    player.setCanControl(true);
                    print("camera done");
                });
            });
        });
    });
}