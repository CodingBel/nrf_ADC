#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/adc.h>

#define SLEEP_TIME_MS   1000

#define ADC_NODE        DT_NODELABEL(adc)       // DT_N_S_soc_S_adc_40007000  "/soc/adc@40007000/"
static const struct device *adc_device = DEVICE_DT_GET(ADC_NODE); 

// configure the ADC 
#define ADC_RESOLUTION          10 
#define ADC_CHANNEL             0
#define ADC_PORT                SAADC_CH_PSELP_PSELP_AnalogInput0       // choos port AN0 i.e. A0_2      
#define ADC_REFERENCE           ADC_REF_INTERNAL                        // Internal ref of 0.6V
#define ADC_GAIN                ADC_GAIN_1_5                            // ADC_REFERENCE*5


// Configure the ADC channel 
struct adc_channel_cfg Zch0_cfg = {
        .gain = ADC_GAIN, 
        .reference = ADC_REFERENCE, 
        .acquisition_time = ADC_ACQ_TIME_DEFAULT, 
        .channel_id = ADC_CHANNEL, 
#ifdef CONFIG_ADC_NRFX_SAADC
        .input_positive = ADC_PORT
#endif
}; 

int16_t sample_buffer[1]; 
struct adc_sequence sequence = {
        // this is where individual channels will be added 
        .channels = BIT(ADC_CHANNEL), 
        .buffer = sample_buffer, 
        // buffer size in bytes, not nubmer of samples 
        .buffer_size = sizeof(sample_buffer), 
        .resolution = ADC_RESOLUTION
};

int main () {
        int err; 

        if (!device_is_ready(adc_device)){
                printk("ADC Device NOT ready \n");
                return; 
        }

        err = adc_channel_setup(adc_device, &Zch0_cfg); 
        if (err != 0){
                printk(" adc_channel_setup failed with error code: %d. \n", err);
                return;  
        }

        while (1){
                err = adc_read(adc_device, &sequence);
                if (err != 0){
                        printk("ADC reading failed with error %d. \n", err);
                        return;  
                } 

                int32_t adc_vref  = adc_ref_internal(adc_device);

                int32_t adc_val = sample_buffer[0]; 
                printk("ADC Raw Value: %d \n", adc_val); 

                adc_raw_to_millivolts(adc_vref, ADC_GAIN, ADC_RESOLUTION, &adc_val);
                printk("ADC in mv: %d mili volts\n", adc_val); 

                k_msleep(SLEEP_TIME_MS);
        }

        return 0; 
}


