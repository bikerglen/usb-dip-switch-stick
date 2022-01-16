using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using HidLibrary;

namespace USB_DIP_SWITCH
{
    public partial class Form1 : Form
    {
        private static HidDevice _device;
        public delegate void ReadHandlerDelegate(HidReport report);

        Button[] buttons = new Button[8];
        bool[] buttonStates = new bool[8];
        Image[] buttonOff = new Image[8];
        Image[] buttonOn = new Image[8];

        public Form1()
        {
            InitializeComponent();
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            buttonOff[0] = USB_DIP_SWITCH.Properties.Resources.brown_off;
            buttonOn[0] = USB_DIP_SWITCH.Properties.Resources.brown_on;
            buttonOff[1] = USB_DIP_SWITCH.Properties.Resources.red_off;
            buttonOn[1] = USB_DIP_SWITCH.Properties.Resources.red_on;
            buttonOff[2] = USB_DIP_SWITCH.Properties.Resources.orange_off;
            buttonOn[2] = USB_DIP_SWITCH.Properties.Resources.orange_on;
            buttonOff[3] = USB_DIP_SWITCH.Properties.Resources.yellow_off;
            buttonOn[3] = USB_DIP_SWITCH.Properties.Resources.yellow_on;
            buttonOff[4] = USB_DIP_SWITCH.Properties.Resources.green_off;
            buttonOn[4] = USB_DIP_SWITCH.Properties.Resources.green_on;
            buttonOff[5] = USB_DIP_SWITCH.Properties.Resources.blue_off;
            buttonOn[5] = USB_DIP_SWITCH.Properties.Resources.blue_on;
            buttonOff[6] = USB_DIP_SWITCH.Properties.Resources.violet_off;
            buttonOn[6] = USB_DIP_SWITCH.Properties.Resources.violet_on;
            buttonOff[7] = USB_DIP_SWITCH.Properties.Resources.gray_off;
            buttonOn[7] = USB_DIP_SWITCH.Properties.Resources.gray_on;

            buttons[0] = button1;
            buttons[1] = button2;
            buttons[2] = button3;
            buttons[3] = button4;
            buttons[4] = button5;
            buttons[5] = button6;
            buttons[6] = button7;
            buttons[7] = button8;

            for (int i = 0; i < 8; i++)
            {
                buttons[i].BackgroundImage = buttonOff[i];
            }

            _device = HidDevices.Enumerate(0x4247, 0x0019).FirstOrDefault();

            if (_device == null)
            {
                MessageBox.Show("Unable to find the USB device.", "Device Not Found", MessageBoxButtons.OK);
            }

            _device.OpenDevice();
            _device.MonitorDeviceEvents = true;
            _device.ReadReport(ReadProcess);

            // send an output report requesting the state of the switches
            var outData = _device.CreateReport();
            outData.ReportId = 0x02;
            outData.Data[0] = 0x55;
            _device.WriteReport(outData);
        }

        private void ReadProcess(HidReport report)
        {
            BeginInvoke(new ReadHandlerDelegate(ReadHandler), new object[] { report });
        }

        private void ReadHandler(HidReport report)
        {
            if (report.ReportId == 0x01)
            {
                int switches = report.Data[0];
                for (int i = 0; i < 8; i++)
                {
                    int mask = 0x80 >> i;
                    buttonStates[i] = ((switches & mask) == mask) ? true : false;
                    UpdateButton(i);
                }
            }
            _device.ReadReport(ReadProcess);
        }

        private void button_Click(object sender, EventArgs e)
        {
            Button button = sender as Button;
            for (int i = 0; i < 8; i++)
            {
                if (button == buttons[i])
                {
                    ToggleButtonState(i);
                }
            }
        }

        private void ToggleButtonState(int i)
        {
            buttonStates[i] = !buttonStates[i];
            UpdateButton(i);
        }

        private void UpdateButton(int i)
        {
            buttons[i].BackgroundImage = buttonStates[i] ? buttonOn[i] : buttonOff[i];
        }
    }
}