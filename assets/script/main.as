Clock@ clock = engine.clock();
Camera@ camera = engine.camera();
Character@ hero = engine.hero();

void main() {
    clock.on("change", "15m", function() {
        print("Clock change");
        print("Camera " + camera.x() + " " + camera.y());
        print("Clock " + clock.year() + "-" + clock.month() + "-" + clock.day() + " " + clock.hour() + ":" + clock.minute());
        print("Hero " + hero.x() + " " + hero.y());

        camera.moveTo(290, 290, function() {
            camera.follow(hero);
            print("camera finished");

            hero.moveTo(290, 290, function() {
                print("move done");
            });
        });
    });
}