void main() {
    Clock@ clock = @engine.clock();
    clock.on("change", "15m", function() {
        Camera@ camera = @engine.camera();
        Clock@ clock = @engine.clock();
        print("Camera " + camera.x() + " " + camera.y());
        print("Clock " + clock.year() + "-" + clock.month() + "-" + clock.day() + " " + clock.hour() + ":" + clock.minute());
    });
}