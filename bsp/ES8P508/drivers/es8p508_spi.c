#include <utils.h>
#include <config.h>
#include <printk.h>
#include <init.h>
#include "HR8P296.h"
#include "lib_config.h"
#include "device.h"
#include "hr8p296_spi.h"

#ifdef configUSING_SPI
#ifdef configUSING_SPI0
// PB20-SCK,PB21-NSS,PB22-SDI,PB23-SDO    FUN1
#define CS_PORT     GPIOB
#define CS_PIN      GPIO_Pin_21

#define SPI_GPIO_AF GPIO_Func_1
#define SCK_PORT    GPIOB
#define SCK_PIN     GPIO_Pin_20
#define SDI_PORT    GPIOB
#define SDI_PIN     GPIO_Pin_22
#define SDO_PORT    GPIOB
#define SDO_PIN     GPIO_Pin_23


static struct hr8p296_spi_bus hr8p296_spi =
{
    .spi = SPI,
};

static err_t configure(struct spi_device *dev, struct spi_cfg *cfg);
static uint32_t xfer(struct spi_device *dev, struct spi_message *msg);

static struct spi_ops hr8p296_spi_ops =
{
    configure,
    xfer
};

static err_t configure(struct spi_device *dev, struct spi_cfg *cfg)
{
    SPI_InitStruType y;

    static const SPI_TYPE_DFS modemap[4] = {SPI_RiseRecFallSend, SPI_RiseSendFallRec, SPI_FallRecRiseSend, SPI_FallSendRiseRec};

    y.SPI_Freq      = cfg->max_hz;
    y.SPI_Df        = modemap[get_bits(cfg->mode, 0, 1)];
    y.SPI_Mode      = SPI_Mode_Master;
    y.SPI_DelayRec  = ENABLE;
    y.SPI_DelaySend = DISABLE;
    y.SPI_SendDelayPeroid = 1;
    SPI_Init(&y);

    SPI_Enable();
    SPI_RecEnable();
    return 0;
};

static uint32_t xfer(struct spi_device *dev, struct spi_message *msg)
{
    struct hr8p296_spi_cs *hr8p296_spi_cs = dev->parent.user_data;
    uint32_t size = msg->len;

    if (msg->cs_take)
        GPIO_ResetBit(hr8p296_spi_cs->GPIOx, hr8p296_spi_cs->GPIO_Pin);

    {
        const uint8_t *send_ptr = msg->send_buf;
        uint8_t *recv_ptr = msg->recv_buf;

        while (size--)
        {
            uint8_t data = 0xFF;

            if (send_ptr) data = *send_ptr++;

            while (SPI_GetFlagStatus(SPI_Flag_RB));
            SPI_SendByte(data);
            while (SPI_GetFlagStatus(SPI_Flag_IDLE) == RESET);
            while (SPI_GetFlagStatus(SPI_Flag_RB) == RESET);
            data = SPI_RecByte();

            if (recv_ptr) *recv_ptr++ = data;
        }
    }

    if (msg->cs_release)
        GPIO_SetBit(hr8p296_spi_cs->GPIOx, hr8p296_spi_cs->GPIO_Pin);

    return msg->len;
};

static void spi_gpio_init(void)
{
    GPIO_InitStruType x;

    GPIO_RegUnLock();
    x.GPIO_Func      = SPI_GPIO_AF;
    x.GPIO_Direction = GPIO_Dir_Out;
    x.GPIO_PDEN      = DISABLE;
    x.GPIO_PUEN      = DISABLE;
    x.GPIO_OD        = DISABLE;
    GPIO_Init(SCK_PORT, SCK_PIN, &x);
    GPIO_Init(SDI_PORT, SDI_PIN, &x);
    GPIO_Init(SDO_PORT, SDO_PIN, &x);
    GPIO_RegLock();
}

err_t hr8p296_add_spi(void)
{
    spi_gpio_init();

    return spi_bus_register(&hr8p296_spi.parent, "spi", &hr8p296_spi_ops);
}
#endif

int board_setup_spi(void)
{
#ifdef configUSING_SPI0
    GPIO_InitStruType x;
    static struct spi_device spi_device;
    static struct hr8p296_spi_cs  spi_cs;

    hr8p296_add_spi();

#define CS_PORT     GPIOB
#define CS_PIN      GPIO_Pin_21
    GPIO_RegUnLock();
    x.GPIO_Func      = GPIO_Func_0;
    x.GPIO_Direction = GPIO_Dir_Out;
    x.GPIO_PDEN      = DISABLE;
    x.GPIO_PUEN      = DISABLE;
    x.GPIO_OD        = DISABLE;
    GPIO_SetBit(CS_PORT, CS_PIN);
    GPIO_Init(CS_PORT, CS_PIN, &x);
    GPIO_RegLock();

    spi_cs.GPIOx    = CS_PORT;
    spi_cs.GPIO_Pin = CS_PIN;
    spi_bus_attach_device(&spi_device, "spi0", "spi", (void *)&spi_cs);
#endif

    return 0;
}
device_initcall(board_setup_spi);
#endif
