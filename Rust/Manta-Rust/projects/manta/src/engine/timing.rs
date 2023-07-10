use std::time;

pub struct Timing {
    pub sys_last_time : time::Instant,
    pub sys_time : time::Instant,

    pub delta_time : f32,
    pub unscaled_delta_time : f32,

    pub time_scale : f32,
    pub elapsed : f32
}

impl Timing {
    pub fn new() -> Timing {
        return Timing { 
            sys_last_time: time::Instant::now(),
            sys_time: time::Instant::now(),

            delta_time: 1f32,
            unscaled_delta_time: 1f32,

            time_scale: 1f32,
            elapsed: 0f32,
        };
    }

    pub fn update(&mut self) {
        self.sys_last_time = self.sys_time;
        self.sys_time = time::Instant::now();
        let elapsed = self.sys_time - self.sys_last_time;

        self.unscaled_delta_time = elapsed.as_secs_f32();
        self.delta_time = self.time_scale * self.unscaled_delta_time;

        self.elapsed += self.delta_time;
    }
}