void main() {
    Clock@ clock = engine.clock();
    clock.on("change", "15m", function() {
        print("Clock change");

        Camera@ camera = engine.camera();
        Clock@ clock = engine.clock();
        Character@ hero = engine.hero();
        print("Camera " + camera.x() + " " + camera.y());
        print("Clock " + clock.year() + "-" + clock.month() + "-" + clock.day() + " " + clock.hour() + ":" + clock.minute());
        print("Hero " + hero.x() + " " + hero.y());

        hero.moveTo(290, 290, function() {
            print("hero finished");
        });
    });
}