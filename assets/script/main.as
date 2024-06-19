Clock@ clock = engine.clock();
Camera@ camera = engine.camera();
Player@ player = engine.player();
Character@ dude = engine.character("dude");
Character@ horse = engine.character("horse");

void cb1()
{
    horse.turnTo("down");
    clock.delay("5m", function() {
        print("Horse finished " + horse.x());

        int x = 18*48;
        if (horse.x() == x) {
            x = 10*48;
        }

        horse.moveTo(x, 9*48, cb1);
    });
}

void main() {
    /*clock.on("change", "5m", function() {
        print("Clock change");
        print("Camera " + camera.x() + " " + camera.y());
        print("Clock " + clock.year() + "-" + clock.month() + "-" + clock.day() + " " + clock.hour() + ":" + clock.minute());
        print("Dude " + dude.x() + " " + dude.y());

        player.setCanControl(false);
        camera.follow(dude, function() {
            print("camera finished");

            dude.moveTo(19*48, 19*48, function() {
                dude.turnTo("down");
                print("move done");
                camera.follow(player.character(), function() {
                    player.setCanControl(true);
                    print("camera done");
                });
            });
        });
    });*/

    clock.on("change", "5m", function() {
        print("Clock change");
        print("Camera " + camera.x() + " " + camera.y());
        print("Clock " + clock.year() + "-" + clock.month() + "-" + clock.day() + " " + clock.hour() + ":" + clock.minute());
        print("Horse " + horse.x() + " " + horse.y());
    });


    cb1();
}