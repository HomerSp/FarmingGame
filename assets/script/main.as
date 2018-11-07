void main() {
    Clock@ clock = @engine.clock();
    clock.on("change", "2wd", function() {
        Clock@ clock = @engine.clock();
        print("TRIGGERED");
        print("Time " + clock.year() + "-" + clock.month() + "-" + clock.day() + " " + clock.hour() + ":" + clock.minute());
    });
}