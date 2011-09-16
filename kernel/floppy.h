#ifndef _FLOPPY_H
#define _FLOPPY_H 1

/*********************************************************
 *                                                       *
 *          Floppy Disk Controller (FDC) driver          *
 *                                                       *
 *********************************************************/

/* FDC registers */
#define FDC_SRA  0x3f0 /* read-only  */   /* Status Register A              */
#define FDC_SRB  0x3f1 /* read-only  */   /* Status Register B              */
#define FDC_DOR  0x3f2                    /* Digital Output Register        */
#define FDC_TDR  0x3f3                    /* Tape Drive Register            */
#define FDC_MSR  0x3f4 /* read_only  */   /* Main Status Register           */
#define FDC_DRS  0x3f4 /* write-only */   /* Datarate Status Register       */
#define FDC_FIFO 0x3f5                    /* FIFO for data                  */
#define FDC_DIR  0x3f7 /* read-only  */   /* Digital Input Register         */
#define FDC_CCR  0x3f7 /* write-only */   /* Configuration Control Register */

/* Commands */
#define FDC_READ_TRACK           2
#define FDC_SPECIFY              3
#define FDC_SENSE_DRIVE_STATUS   4
#define FDC_WRITE_DATA           5
#define FDC_READ_DATA            6
#define FDC_RECALIBRATE          7
#define FDC_SENSE_INTERRUPT      8
#define FDC_WRITE_DELETED_DATA   9
#define FDC_READ_ID             10
#define FDC_READ_DELETED_DATA   12
#define FDC_FORMAT_TRACK        13
#define FDC_SEEK                15
#define FDC_VERSION             16
#define FDC_SCAN_EQUAL          17
#define FDC_PERPENDICULAR_MODE  18
#define FDC_CONFIGURE           19
#define FDC_VERIFY              22
#define FDC_SCAN_LOW_OR_EQUAL   25
#define FDC_SCAN_HIGH_OR_EQUAL  29

void floppy_init(void);

#endif /* floppy.h */
