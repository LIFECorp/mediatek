#include <asm/mach/time.h>
#include <mach/mt_timer.h>

extern struct mt_clock mt6571_gpt;
extern int generic_timer_register(void);


struct mt_clock *mt6571_clocks[] =
{
    &mt6571_gpt,
};

static void __init mt6571_timer_init(void)
{
    int i;
    struct mt_clock *clock;
    int err;

    for (i = 0; i < ARRAY_SIZE(mt6571_clocks); i++) {
        clock = mt6571_clocks[i];

        clock->init_func();

        if (clock->clocksource.name) {
            err = clocksource_register(&(clock->clocksource));
            if (err) {
                pr_err("mt6571_timer_init: clocksource_register failed for %s\n", clock->clocksource.name);
            }
        }

        err = setup_irq(clock->irq.irq, &(clock->irq));
        if (err) {
            pr_err("mt6571_timer_init: setup_irq failed for %s\n", clock->irq.name);
        }

        if (clock->clockevent.name)
            clockevents_register_device(&(clock->clockevent));
    }

    err = generic_timer_register(); 
    if (err) {
        pr_err("generic_timer_register failed, err=%d\n", err);
    }
}


struct sys_timer mt6571_timer = {
    .init = mt6571_timer_init,
};
