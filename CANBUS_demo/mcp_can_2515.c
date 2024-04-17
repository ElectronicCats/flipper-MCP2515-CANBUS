#include "mcp_can_2515.h"

// To read a register with SPI protocol
static bool read_register(FuriHalSpiBusHandle* spi, uint8_t address, uint8_t* data) {
    bool ret = true;
    uint8_t instruction[] = {INSTRUCTION_READ, address};
    furi_hal_spi_acquire(spi);
    ret =
        (furi_hal_spi_bus_tx(spi, instruction, sizeof(instruction), TIMEOUT_SPI) &&
         furi_hal_spi_bus_rx(spi, data, sizeof(data), TIMEOUT_SPI));

    furi_hal_spi_release(spi);
    return ret;
}

// To read a register with SPI Protocol
static bool set_register(FuriHalSpiBusHandle* spi, uint8_t address, uint8_t data) {
    bool ret = true;
    uint8_t instruction[] = {INSTRUCTION_WRITE, address, data};
    furi_hal_spi_acquire(spi);
    ret = furi_hal_spi_bus_tx(spi, instruction, sizeof(instruction), TIMEOUT_SPI);
    furi_hal_spi_release(spi);
    return ret;
}

// To write multiple registers
static bool
    set_registerS(FuriHalSpiBusHandle* spi, uint8_t address, uint8_t* data, uint8_t count) {
    bool ret = true;
    uint8_t instruction[] = {INSTRUCTION_WRITE, address};

    for(uint8_t i = 0; i < count; i++) {
        instruction[i + 2] = data[i];
    }

    furi_hal_spi_acquire(spi);
    ret = furi_hal_spi_bus_tx(spi, instruction, count + 2, TIMEOUT_SPI);
    furi_hal_spi_release(spi);
    return ret;
}

// To modify the value of one bit from a register
static bool
    modify_register(FuriHalSpiBusHandle* spi, uint8_t address, uint8_t mask, uint8_t data) {
    uint8_t instruction[] = {INSTRUCTION_BITMOD, address, mask, data};
    bool ret = true;
    furi_hal_spi_acquire(spi);
    ret = furi_hal_spi_bus_tx(spi, instruction, sizeof(instruction), TIMEOUT_SPI);
    furi_hal_spi_release(spi);
    return ret;
}

// To reset the MCP2515
static bool mcp_reset(FuriHalSpiBusHandle* spi) {
    uint8_t buff[1] = {INSTRUCTION_RESET};
    bool ret = true;
    furi_hal_spi_acquire(spi);
    ret = furi_hal_spi_bus_tx(spi, buff, sizeof(buff), TIMEOUT_SPI);
    furi_hal_spi_release(spi);
    return ret;
}

// To get the MCP2515 status
bool mcp_get_status(FuriHalSpiBusHandle* spi, uint8_t* data) {
    uint8_t buff[1] = {INSTRUCTION_READ_STATUS};
    bool ret = true;
    furi_hal_spi_acquire(spi);
    ret =
        (furi_hal_spi_bus_tx(spi, buff, sizeof(buff), TIMEOUT_SPI) &&
         furi_hal_spi_bus_rx(spi, data, sizeof(data), TIMEOUT_SPI));
    furi_hal_spi_release(spi);
    return ret;
}

// To set the Mode
static bool set_mode(FuriHalSpiBusHandle* spi, MCP_MODE newmode) {
    bool ret = true;
    static uint8_t readStatus = 0;

    modify_register(spi, MCP_CANCTRL, CANCTRL_REQOP, newmode);

    read_register(spi, MCP_CANSTAT, &readStatus);

    readStatus &= CANSTAT_OPM;

    if(readStatus == newmode)
        ret = true;
    else
        ret = false;
    return ret;
}

// To set Config mode
bool set_config_mode(MCP2515* mcp_can) {
    FuriHalSpiBusHandle* spi = mcp_can->spi;
    bool ret = true;
    ret = set_mode(spi, MODE_CONFIG);
    return ret;
}

// To set Normal Mode
bool set_normal_mode(MCP2515* mcp_can) {
    FuriHalSpiBusHandle* spi = mcp_can->spi;
    bool ret = true;
    ret = set_mode(spi, MCP_NORMAL);
    return ret;
}

//To set ListenOnly Mode
bool set_listen_only_mode(MCP2515* mcp_can) {
    FuriHalSpiBusHandle* spi = mcp_can->spi;
    bool ret = true;
    ret = set_mode(spi, MCP_LISTENONLY);
    return ret;
}

// To set Sleep Mode
bool set_sleep_mode(MCP2515* mcp_can) {
    FuriHalSpiBusHandle* spi = mcp_can->spi;
    bool ret = true;
    ret = set_mode(spi, MCP_SLEEP);
    return ret;
}

// To set LoopBackMode
bool set_loop_back_mode(MCP2515* mcp_can) {
    FuriHalSpiBusHandle* spi = mcp_can->spi;
    bool ret = true;
    ret = set_mode(spi, MCP_LOOPBACK);
    return ret;
}

// To init the buffer
static void write_mf(FuriHalSpiBusHandle* spi, uint8_t adress, uint8_t ext, uint8_t id) {
    uint16_t canId = (uint16_t)(id & 0x0FFFF);
    uint8_t bufData[4];

    if(ext) {
        bufData[MCP_EID0] = (uint8_t)(canId & 0xFF);
        bufData[MCP_EID8] = (uint8_t)(canId >> 8);
        canId = (uint16_t)(id >> 16);
        bufData[MCP_SIDL] = (uint8_t)(canId & 0x03);
        bufData[MCP_SIDL] += (uint8_t)((canId & 0x1C) << 3);
        bufData[MCP_SIDL] |= MCP_TXB_EXIDE_M;
        bufData[MCP_SIDH] = (uint8_t)(canId >> 5);
    } else {
        bufData[MCP_EID0] = (uint8_t)(canId & 0xFF);
        bufData[MCP_EID8] = (uint8_t)(canId >> 8);
        canId = (uint16_t)(id >> 16);
        bufData[MCP_SIDL] = (uint8_t)((canId & 0x07) << 5);
        bufData[MCP_SIDH] = (uint8_t)(canId >> 3);
    }

    set_registerS(spi, adress, bufData, 8);
}

// This function works to get the Can Id from the buffer
static void read_Id(FuriHalSpiBusHandle* spi, uint8_t addr, uint32_t* id, uint8_t* ext) {
    uint8_t tbufdata[4] = {0, 0, 0, 0};
    *ext = 0;
    *id = 0;

    read_register(spi, addr, tbufdata);

    *id = (tbufdata[MCP_SIDH] << 3) + (tbufdata[MCP_SIDL] >> 5);

    if((tbufdata[MCP_SIDL] & MCP_TXB_EXIDE_M) == MCP_TXB_EXIDE_M) {
        *id = (*id << 2) + (tbufdata[MCP_SIDL] & 0x03);
        *id = (*id << 8) + tbufdata[MCP_EID8];
        *id = (*id << 8) + tbufdata[MCP_EID0];
        *ext = 1;
    }
}

// This function Works to read the can frame from the buffer
static void read_canframe(FuriHalSpiBusHandle* spi, const uint8_t addr, CANFRAME* frame) {
    uint8_t ctrl = 0, len = 0;
    static uint8_t data = 0;

    read_Id(spi, addr, &frame->canId, &frame->ext);

    read_register(spi, addr - 1, &ctrl);

    read_register(spi, addr + 4, &(len));

    if(ctrl & 0x08)
        frame->req = 1;
    else
        frame->req = 0;

    frame->data_lenght = len;

    len &= MCP_DLC_MASK;

    for(uint8_t i = 0; i < len; i++) {
        read_register(spi, addr + 5 + i, &data);
        frame->buffer[i] = data;
    }
}

// This function works to init the buffers in the mcp2515
static void init_can_buffer(FuriHalSpiBusHandle* spi) {
    uint8_t a1 = 0, a2 = 0, a3 = 0;

    uint8_t std = 0;
    uint8_t ext = 1;

    uint32_t ulMask = 0x00, ulFilt = 0x00;

    // ulMask ---------------------------

    write_mf(spi, MCP_RXM0SIDH, ext, ulMask);

    write_mf(spi, MCP_RXM1SIDH, ext, ulMask);

    // ulFilt --------------------------

    write_mf(spi, MCP_RXF0SIDH, ext, ulFilt);

    write_mf(spi, MCP_RXF1SIDH, std, ulFilt);

    write_mf(spi, MCP_RXF2SIDH, ext, ulFilt);

    write_mf(spi, MCP_RXF3SIDH, std, ulFilt);

    write_mf(spi, MCP_RXF4SIDH, ext, ulFilt);

    write_mf(spi, MCP_RXF5SIDH, std, ulFilt);

    a1 = MCP_TXB0CTRL;
    a2 = MCP_TXB1CTRL;
    a3 = MCP_TXB2CTRL;

    for(int i = 0; i < 14; i++) {
        set_register(spi, a1, 0);
        set_register(spi, a2, 0);
        set_register(spi, a3, 0);
        a1++;
        a2++;
        a3++;
    }

    set_register(spi, MCP_RXB0CTRL, 0);
    set_register(spi, MCP_RXB1CTRL, 0);
}

// This function works to set Registers to initialize the MCP2515
static void set_registers_init(FuriHalSpiBusHandle* spi) {
    set_register(spi, MCP_CANINTE, MCP_RX0IF | MCP_RX1IF);

    set_register(spi, MCP_BFPCTRL, MCP_BxBFS_MASK | MCP_BxBFE_MASK);

    set_register(spi, MCP_TXRTSCTRL, 0x00);
}

// This function Works to set the Clock and Bitrate of the MCP2515
static void mcp_set_bitrate(FuriHalSpiBusHandle* spi, MCP_BITRATE bitrate, MCP_CLOCK clk) {
    uint8_t cfg1 = 0, cfg2 = 0, cfg3 = 0;

    switch(clk) {
    case MCP_8MHZ:
        switch(bitrate) {
        case MCP_125KPS:
            cfg1 = MCP_8MHz_125kBPS_CFG1;
            cfg2 = MCP_8MHz_125kBPS_CFG2;
            cfg3 = MCP_8MHz_125kBPS_CFG3;
            break;
        case MCP_250KBPS:
            cfg1 = MCP_8MHz_250kBPS_CFG1;
            cfg2 = MCP_8MHz_250kBPS_CFG2;
            cfg3 = MCP_8MHz_250kBPS_CFG3;
            break;
        case MCP_500KBPS:
            cfg1 = MCP_8MHz_500kBPS_CFG1;
            cfg2 = MCP_8MHz_500kBPS_CFG2;
            cfg3 = MCP_8MHz_500kBPS_CFG3;
            break;
        case MCP_1000KBPS:
            cfg1 = MCP_8MHz_1000kBPS_CFG1;
            cfg2 = MCP_8MHz_1000kBPS_CFG2;
            cfg3 = MCP_8MHz_1000kBPS_CFG3;
            break;
        }
        break;
    case MCP_16MHZ:
        switch(bitrate) {
        case MCP_125KPS:
            cfg1 = MCP_16MHz_125kBPS_CFG1;
            cfg2 = MCP_16MHz_125kBPS_CFG2;
            cfg3 = MCP_16MHz_125kBPS_CFG3;
            break;
        case MCP_250KBPS:
            cfg1 = MCP_16MHz_250kBPS_CFG1;
            cfg2 = MCP_16MHz_250kBPS_CFG2;
            cfg3 = MCP_16MHz_250kBPS_CFG3;
            break;
        case MCP_500KBPS:
            cfg1 = MCP_16MHz_500kBPS_CFG1;
            cfg2 = MCP_16MHz_500kBPS_CFG2;
            cfg3 = MCP_16MHz_500kBPS_CFG3;
            break;
        case MCP_1000KBPS:
            cfg1 = MCP_16MHz_1000kBPS_CFG1;
            cfg2 = MCP_16MHz_1000kBPS_CFG2;
            cfg3 = MCP_16MHz_1000kBPS_CFG3;
            break;
        }
        break;

    case MCP_20MHZ:
        switch(bitrate) {
        case MCP_125KPS:
            cfg1 = MCP_20MHz_125kBPS_CFG1;
            cfg2 = MCP_20MHz_125kBPS_CFG2;
            cfg3 = MCP_20MHz_125kBPS_CFG3;
            break;
        case MCP_250KBPS:
            cfg1 = MCP_20MHz_250kBPS_CFG1;
            cfg2 = MCP_20MHz_250kBPS_CFG2;
            cfg3 = MCP_20MHz_250kBPS_CFG3;
            break;
        case MCP_500KBPS:
            cfg1 = MCP_20MHz_500kBPS_CFG1;
            cfg2 = MCP_20MHz_500kBPS_CFG2;
            cfg3 = MCP_20MHz_500kBPS_CFG3;
            break;
        case MCP_1000KBPS:
            cfg1 = MCP_20MHz_1000kBPS_CFG1;
            cfg2 = MCP_20MHz_1000kBPS_CFG2;
            cfg3 = MCP_20MHz_1000kBPS_CFG3;
            break;
        }
        break;
    }

    set_register(spi, MCP_CNF1, cfg1);
    set_register(spi, MCP_CNF2, cfg2);
    set_register(spi, MCP_CNF3, cfg3);
}

// This function Works to get the Can message
ERROR_CAN read_can_message(MCP2515* mcp_can, CANFRAME* frame) {
    FuriHalSpiBusHandle* spi = mcp_can->spi;
    ERROR_CAN ret = ERROR_OK;
    static uint8_t status = 0;

    mcp_get_status(spi, &status);

    if(status & MCP_STAT_RX0IF) {
        read_canframe(spi, MCP_RXB0SIDH, frame);
        modify_register(spi, MCP_CANINTF, MCP_RX0IF, 0);
    }

    else if(status & MCP_STAT_RX1IF) {
        read_canframe(spi, MCP_RXB1SIDH, frame);
        modify_register(spi, MCP_CANINTF, MCP_RX1IF, 0);
    } else {
        ret = ERROR_NOMSG;
    }

    return ret;
}

// This function return the error in the can bus network
uint8_t get_error(MCP2515* mcp_can) {
    FuriHalSpiBusHandle* spi = mcp_can->spi;
    uint8_t err = 0;
    read_register(spi, MCP_EFLG, &err);
    modify_register(spi, MCP_EFLG, MCP_EFLG_RX0OVR, 0);
    modify_register(spi, MCP_EFLG, MCP_EFLG_RX1OVR, 0);
    return err;
}

// This function works to check if there is an error in the CANBUS network
ERROR_CAN check_error(MCP2515* mcp_can) {
    FuriHalSpiBusHandle* spi = mcp_can->spi;

    uint8_t eflg = 0;

    read_register(spi, MCP_EFLG, &eflg);

    if(eflg & MCP_EFLG_ERRORMASK) {
        return ERROR_FAIL;
    } else {
        return ERROR_OK;
    }
}

// This function works to get
ERROR_CAN check_receive(MCP2515* mcp_can) {
    FuriHalSpiBusHandle* spi = mcp_can->spi;
    static uint8_t status = 0;
    mcp_get_status(spi, &status);

    if(status & MCP_STAT_RXIF_MASK)
        return ERROR_OK;
    else
        return ERROR_NOMSG;
}

// Function to get a free Buffer
static ERROR_CAN get_next_buffer(FuriHalSpiBusHandle* spi, uint8_t* buffer) {
    static uint8_t ctrl_reg[3] = {MCP_TXB0CTRL, MCP_TXB1CTRL, MCP_TXB2CTRL};
    uint8_t ctrl_value;
    *buffer = 0x00;

    for(uint8_t i = 0; i < 3; i++) {
        uint8_t register_buff = ctrl_reg[i];
        read_register(spi, register_buff, &(ctrl_value));

        log_info("Buffer is_free: %u ", ctrl_reg[i]);

        if((ctrl_value & MCP_TXB_TXREQ_M) == 0) {
            *buffer = ctrl_reg[i] + 1;
            return ERROR_OK;
        }
    }
    return ERROR_ALLTXBUSY;
}

static void write_id(FuriHalSpiBusHandle* spi, uint8_t addr, uint32_t can_id, uint8_t extension) {
    uint16_t id;
    uint8_t t_buf_data[4];

    id = (uint16_t)(can_id & 0x0FFFF);

    if(extension == 1) {
        t_buf_data[MCP_EID0] = (uint8_t)(id & 0xFF);
        t_buf_data[MCP_EID8] = (uint8_t)(id >> 8);
        id = (uint16_t)(id >> 16);
        t_buf_data[MCP_SIDL] = (uint8_t)(id & 0x03);
        t_buf_data[MCP_SIDL] += (uint8_t)((id & 0x1C) << 3);
        t_buf_data[MCP_SIDL] |= MCP_TXB_EXIDE_M;
        t_buf_data[MCP_SIDH] = (uint8_t)(id >> 5);
    } else {
        t_buf_data[MCP_SIDH] = (uint8_t)(id >> 3);
        t_buf_data[MCP_SIDL] = (uint8_t)((id & 0x07) << 5);
        t_buf_data[MCP_EID0] = 0;
        t_buf_data[MCP_EID8] = 0;
    }

    for(uint8_t i = 0; i < 4; i++) {
        set_register(spi, addr + i, t_buf_data[i]);
    }
}

static void write_bytes(FuriHalSpiBusHandle* spi, CANFRAME* frame, uint8_t buffer_number) {
    uint8_t data_lenght = frame->data_lenght;
    uint8_t data[data_lenght];

    UNUSED(spi);
    log_info("Direccion starts in: %u", buffer_number);

    for(uint8_t i = 0; i < data_lenght; i++) {
        data[i] = frame->buffer[i];
    }

    for(uint8_t i = 0; i < data_lenght; i++) {
        set_register(spi, buffer_number + i, data[i]);
    }
}

static void write_can_message(FuriHalSpiBusHandle* spi, CANFRAME* frame, uint8_t buffer_number) {
    uint8_t data_lenght = frame->data_lenght;
    uint8_t data_register_lenght = data_lenght;
    uint8_t ext = frame->ext;

    write_bytes(spi, frame, buffer_number + 5);

    if(ext == 1) data_register_lenght |= MCP_RTR_MASK;

    set_register(spi, buffer_number + 4, data_register_lenght);

    write_id(spi, buffer_number, frame->canId, ext);
}

static ERROR_CAN configure_can_message(FuriHalSpiBusHandle* spi, CANFRAME* frame) {
    ERROR_CAN ret = ERROR_OK;
    uint8_t buf_n_free, res, res1;
    uint32_t u_time_out = 0;
    uint8_t data_lenght = frame->data_lenght;

    if(data_lenght > 8) return ERROR_FAILTX;

    res = get_next_buffer(spi, &buf_n_free);
    if(res != ERROR_OK) return res;

    modify_register(spi, buf_n_free - 1, MCP_TXB_TXREQ_M, 0);

    write_can_message(spi, frame, buf_n_free);

    modify_register(spi, buf_n_free - 1, MCP_TXB_TXREQ_M, MCP_TXB_TXREQ_M);

    do {
        read_register(spi, buf_n_free - 1, &(res1));
        res1 = res1 & 0x08;
        furi_delay_us(1);
        u_time_out++;
    } while(res1 && (u_time_out < TIMEOUTVALUE));

    if(u_time_out >= TIMEOUTVALUE) return ERROR_SEND_MSG_TIMEOUT;

    return ret;
}

// This function works to send only the frame
ERROR_CAN send_can_frame(MCP2515* mcp_can, CANFRAME* frame) {
    FuriHalSpiBusHandle* spi = mcp_can->spi;
    return configure_can_message(spi, frame);
}

// This function works to alloc the struct
MCP2515* mcp_alloc(MCP_MODE mode, MCP_CLOCK clck, MCP_BITRATE bitrate) {
    MCP2515* mcp_can = malloc(sizeof(MCP2515));
    mcp_can->spi = spi_alloc();
    mcp_can->mode = mode;
    mcp_can->bitRate = bitrate;
    mcp_can->clck = clck;
    return mcp_can;
}

// To free
void free_mcp2515(MCP2515* mcp_can) {
    mcp_reset(mcp_can->spi);
    furi_hal_spi_bus_handle_deinit(mcp_can->spi);
}

// This function starts the SPI communication and set the MCP2515 device
static ERROR_CAN mcp2515_start(MCP2515* mcp_can) {
    furi_hal_spi_bus_handle_init(mcp_can->spi);

    bool ret = true;

    mcp_reset(mcp_can->spi);

    mcp_set_bitrate(mcp_can->spi, mcp_can->bitRate, mcp_can->clck);

    init_can_buffer(mcp_can->spi);

    set_registers_init(mcp_can->spi);

    ret = set_mode(mcp_can->spi, mcp_can->mode);
    if(!ret) return ERROR_FAILINIT;

    return ERROR_OK;
}

// Init the mcp2515 device
ERROR_CAN mcp2515_init(MCP2515* mcp_can) {
    return mcp2515_start(mcp_can);
}
