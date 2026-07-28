#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/uart.h>

#include <zephyr/sys/util.h>
#include <inttypes.h>

#include <zephyr/timing/timing.h>

//#define DEBUG

// Led pin configurations
static const struct gpio_dt_spec red = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
static const struct gpio_dt_spec green = GPIO_DT_SPEC_GET(DT_ALIAS(led1), gpios);
//static const struct gpio_dt_spec blue = GPIO_DT_SPEC_GET(DT_ALIAS(led2), gpios);

// Red led thread initialization
#define STACKSIZE 500
#define PRIORITY 5
//void red_led_task(void *, void *, void*);
//void yellow_led_task(void *, void *, void*);
//void green_led_task(void *, void *, void*);
void led_task(void*, void*, void*);

int init_led();
int init_button();

static void uart_task(void *unused1, void *unused2, void *unused3);
static void dispatcher_task(void *unused1, void *unused2, void *unused3);

// UART initialization
#define UART_DEVICE_NODE DT_CHOSEN(zephyr_shell_uart)
static const struct device *const uart_dev = DEVICE_DT_GET(UART_DEVICE_NODE);

// Create dispatcher FIFO buffer
K_FIFO_DEFINE(liikennevalo_fifo);

// FIFO dispatcher data type
struct data_t {
	void *fifo_reserved;
	char msg[20];
};

int init_uart(void)
{
	// UART initialization
	if (!device_is_ready(uart_dev)) {
		return 1;
	} 
	return 0;
}


//K_THREAD_DEFINE(red_thread,STACKSIZE,red_led_task,NULL,NULL,NULL,PRIORITY,0,0);
//K_THREAD_DEFINE(yellow_thread,STACKSIZE,yellow_led_task,NULL,NULL,NULL,PRIORITY,0,0);
//K_THREAD_DEFINE(yellow_thread,STACKSIZE,yellow_led_task,NULL,NULL,NULL,PRIORITY,0,0);
K_THREAD_DEFINE(led_thread,STACKSIZE,led_task,NULL,NULL,NULL,PRIORITY,0,0);

K_THREAD_DEFINE(dis_thread,STACKSIZE,dispatcher_task,NULL,NULL,NULL,PRIORITY,0,0);
K_THREAD_DEFINE(uart_thread,STACKSIZE,uart_task,NULL,NULL,NULL,PRIORITY,0,0);

int led_state = 0;

int release = 0;

int paused = 0;

//Configure buttons
#define BUTTON_0 DT_ALIAS(sw0)
#define BUTTON_1 DT_ALIAS(sw1)
// #define BUTTON_1 DT_ALIAS(sw1)
static const struct gpio_dt_spec button_0 = GPIO_DT_SPEC_GET_OR(BUTTON_0, gpios, {0});
static struct gpio_callback button_0_data;

static const struct gpio_dt_spec button_1 = GPIO_DT_SPEC_GET_OR(BUTTON_1, gpios, {1});
static struct gpio_callback button_1_data;


// Main program
int main(void)
{
	timing_init();
	timing_start();

	timing_t start_time = timing_counter_get();

	init_led();

	int ret = init_button();
	if (ret < 0) {
		return 0;
	}

	ret = init_uart();
	if (ret != 0) {
		printk("UART initialization failed!\n");
		return ret;
	}
	
	timing_t end_time = timing_counter_get();
	timing_stop();
	uint64_t timing_ns = timing_cycles_to_ns(timing_cycles_get(&start_time, &end_time));
	printk("Main: %lld\n", timing_ns);

	while (1) {
		k_msleep(10); // sleep 10ms
	}

	return 0;
}

static void uart_task(void *unused1, void *unused2, void *unused3)
{
	timing_init();
	timing_start();

	timing_t start_time = timing_counter_get();

	// Received character from UART
	char rc=0;
	// Message from UART
	char uart_msg[20];
	memset(uart_msg,0,20);
	int uart_msg_cnt = 0;

	while (true) {
		if (uart_poll_in(uart_dev,&rc) == 0) {
			if (rc != '\r') {
				uart_msg[uart_msg_cnt] = rc;
				uart_msg_cnt++;
			} else {
				printk("UART msg: %s\n", uart_msg);

				struct data_t *buf = k_malloc(sizeof(struct data_t));
				if (buf == NULL) {
					return;
				}

				snprintf(buf->msg, 20, "%s", uart_msg);
				//*buf->msg = uart_msg;

				k_fifo_put(&liikennevalo_fifo, buf);

				uart_msg_cnt = 0;

				timing_t end_time = timing_counter_get();
				timing_stop();
				uint64_t timing_ns = timing_cycles_to_ns(timing_cycles_get(&start_time, &end_time));
				printk("UART Task: %lld\n", timing_ns);
				//memset(uart_msg,0,20);
			}
		}
		//printk("waiting for data\n");
		k_msleep(10);
	}
}

static void dispatcher_task(void *unused1, void *unused2, void *unused3)
{
	timing_start();
	timing_t start_time = timing_counter_get();
	printk("dispatcher task started\n");
	while (true) {
		struct data_t *rec_item = k_fifo_get(&liikennevalo_fifo, K_FOREVER);
		char sequence[20];
		memcpy(sequence,rec_item->msg,20);
		k_free(rec_item);

		printk("Dispatcher: %s\n", sequence);
		for(int i=0;i<20;i++) {
#ifdef DEBUG
			printk("dispatcher task\n");
#endif
			switch(sequence[i]) {
				case 'R':
					led_state=1;
					break;
				case 'Y':
					led_state=2;
					break;
				case 'G':
					led_state=3;
					break;
				case '\0':
					led_state = 4;
					break;
			}
			release = 0;
			while (!release) {
				k_yield();
			}
		}
		timing_t end_time = timing_counter_get();
		timing_stop();
		uint64_t timing_ns = timing_cycles_to_ns(timing_cycles_get(&start_time, &end_time));
		printk("Dispatcher task: %lld\n", timing_ns);
		k_yield();
	}
}


// Initialize leds
int init_led()
{
	timing_start();
	timing_init();
	timing_t start_time = timing_counter_get();
	// Led pin initialization
	int ret = gpio_pin_configure_dt(&red, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		printk("Error: Red Led configure failed\n");		
		return ret;
	}
	ret = gpio_pin_configure_dt(&green, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		printk("Error: Green Led configure failed\n");		
		return ret;
	}

	// set led off
	gpio_pin_set_dt(&red,0);
	gpio_pin_set_dt(&green,0);

	timing_t end_time = timing_counter_get();
	timing_stop();
	uint64_t timing_ns = timing_cycles_to_ns(timing_cycles_get(&start_time, &end_time));
	printk("Led initialized ok %lld\n", timing_ns);

	return 0;
}


void led_task(void *, void *, void*)
{
	timing_start();
	timing_init();
	printk("Led task started\n");

	uint64_t total_time = 0;

	bool running = true;

	while (running) {
		timing_t start_time = timing_counter_get();
		switch(led_state) {
			case 1:
				gpio_pin_set_dt(&red,1);
#ifdef DEBUG
				printk("Red on\n");
#endif
				goto sleep;
			case 2:
				gpio_pin_set_dt(&red,1);
#ifdef DEBUG
				printk("Red on\n");
#endif
				gpio_pin_set_dt(&green,1);
#ifdef DEBUG
				printk("Green on\n");
#endif
				goto sleep;
			case 3:
				gpio_pin_set_dt(&green,1);
#ifdef DEBUG
				printk("Green on\n");
#endif
				goto sleep;
			sleep:
				timing_t end_time = timing_counter_get();
				uint64_t timing_ns = timing_cycles_to_ns(timing_cycles_get(&start_time, &end_time));
#ifdef DEBUG
				printk("Led time %lld\n", timing_ns);
#endif
				total_time += timing_ns;
				k_sleep(K_SECONDS(1));
				if (!paused) {
					gpio_pin_set_dt(&red,0);
					gpio_pin_set_dt(&green,0);
				}
				led_state = 0;
				release = 1;
				break;
			case 4:
				running = false;
			default:
				k_yield();
				break;
		}
	}
	printk("LED Task total time: %lld\n", total_time);
	timing_stop();
}
// Button interrupt handler
void button_0_handler(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	paused = 0;
	printk("Button 0 pressed\n");
}
// Button interrupt handler
void button_1_handler(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	paused = 1;
	printk("Button 1 pressed\n");
}

// Button initialization
int init_button()
{
	int ret;
	if (!gpio_is_ready_dt(&button_0)) {
		printk("Error: button 0 is not ready\n");
		return -1;
	}

	ret = gpio_pin_configure_dt(&button_0, GPIO_INPUT);
	if (ret != 0) {
		printk("Error: failed to configure pin\n");
		return -1;
	}

	ret = gpio_pin_interrupt_configure_dt(&button_0, GPIO_INT_EDGE_TO_ACTIVE);
	if (ret != 0) {
		printk("Error: failed to configure interrupt on pin\n");
		return -1;
	}

	gpio_init_callback(&button_0_data, button_0_handler, BIT(button_0.pin));
	gpio_add_callback(button_0.port, &button_0_data);
	printk("Set up button 0 ok\n");

	if (!gpio_is_ready_dt(&button_1)) {
		printk("Error: button 1 is not ready\n");
		return -1;
	}

	ret = gpio_pin_configure_dt(&button_1, GPIO_INPUT);
	if (ret != 0) {
		printk("Error: failed to configure pin\n");
		return -1;
	}

	ret = gpio_pin_interrupt_configure_dt(&button_1, GPIO_INT_EDGE_TO_ACTIVE);
	if (ret != 0) {
		printk("Error: failed to configure interrupt on pin\n");
		return -1;
	}

	gpio_init_callback(&button_1_data, button_1_handler, BIT(button_1.pin));
	gpio_add_callback(button_1.port, &button_1_data);
	printk("Set up button 1 ok\n");

	return 0;
}
