void main() {
    ClockListenerArg arg;
    arg.minute = 15;
    clock.on("change", arg, function() {
        print("TRIGGERED");
    });
    print("Time " + clock.year() + "-" + clock.month() + "-" + clock.day() + " " + clock.hour() + ":" + clock.minute() + " - " + arg.hour + " " + arg.minute);
}