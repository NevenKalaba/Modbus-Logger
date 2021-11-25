#include <stdio.h>
#include <unistd.h>
#include <modbus.h>

#define SERVER_ID		1
#define COIL_ADDRESS		1
#define BCM_PIN_DE		22

int main()
{

	modbus_t *ctx;
	ctx = modbus_new_rtu("/dev/ttyAMA0", 9600, 'N', 8, 1);

	if (ctx == NULL)
	{
	fprintf(stderr, "Unable to create the libmodbus context!\n");
	return -1;
	}
	
	modbus_set_slave(ctx, SERVER_ID);
	modbus_set_debug(ctx,TRUE);
	
	if (modbus_connect(ctx) == -1)
	{

	fprintf(stderr, "Connection failed.\n");
	modbus_free(ctx);
	return -1;

	}

	modbus_enable_rpi(ctx, TRUE);
	modbus_configure_rpi_bcm_pin(ctx, BCM_PIN_DE);
	modbus_rpi_pin_export_direction(ctx);

	sleep(1);
	// TODO: Add your code here
	modbus_write_bit(ctx,COIL_ADDRESS, TRUE);
	sleep(5);
	modbus_write_bit(ctx,COIL_ADDRESS, FALSE);


	modbus_rpi_pin_unexport_direction(ctx);
	modbus_close(ctx);
	modbus_free(ctx);
	return 0;
}
