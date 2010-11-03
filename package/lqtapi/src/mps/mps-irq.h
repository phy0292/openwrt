
#define MPS_IRQ_BASE 160
#define MPS_IRQ(x) (MPS_IRQ_BASE + (x))
#define MPS_IRQ_CMD_UPSTREAM MPS_IRQ(0)
#define MPS_IRQ_DATA_UPSTREAM MPS_IRQ(1)
#define MPS_IRQ_CMD_DOWNSTREAM MPS_IRQ(2)
#define MPS_IRQ_DATA_DOWNSTREAM MPS_IRQ(3)
#define MPS_IRQ_RCV_OVERFLOW MPS_IRQ(4)
#define MPS_IRQ_EVENT MPS_IRQ(5)
#define MPS_IRQ_EVENT_OVERFLOW MPS_IRQ(6)
#define MPS_IRQ_CMD_ERROR MPS_IRQ(8)
#define MPS_IRQ_PCM_CRASH MPS_IRQ(9)
#define MPS_IRQ_DATA_ERROR MPS_IRQ(10)
#define MPS_IRQ_MIPS_OL MPS_IRQ(11)
#define MPS_IRQ_WATCHDOG_FAIL MPS_IRQ(14)
#define MPS_IRQ_DOWNLOAD_DONE MPS_IRQ(15)


