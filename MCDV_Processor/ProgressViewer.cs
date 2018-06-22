using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace MCDV_Processor
{
    public class ProgressViewer
    {
        public float percent = 0.0f;
        public string title = "";

        private int console_y = 0;


        public void Draw(bool end = false, bool error = false)
        {
            int prev_y = Console.CursorTop;

            Console.ForegroundColor = ConsoleColor.Black;
            Console.SetCursorPosition(0, console_y);

            //Get the amount of characters needed to fill region

            string new_title = title + " - " + (int)(percent * 100.0f) + "%";

            int cwidth = Console.BufferWidth;

            int textWidth = new_title.Count();

            int padWidth = (cwidth - textWidth) / 2;

            string newt = "";

            for (int i = 0; i < padWidth; i++)
                newt += " ";

            newt += new_title;

            for (int i = 0; i < cwidth - (padWidth + textWidth); i++)
                newt += " ";

            int percentBuffer = (int)(percent * (float)cwidth);
            percentBuffer = percentBuffer > cwidth ? cwidth : percentBuffer;

            for (int i = 0; i < newt.Length; i++)
            {

                Console.BackgroundColor = ConsoleColor.Green;

                if (i > percentBuffer)
                    Console.BackgroundColor = ConsoleColor.DarkGreen;

                if (end)
                    Console.BackgroundColor = ConsoleColor.DarkGray;

                if (error)
                    Console.BackgroundColor = ConsoleColor.DarkRed;

                Console.Write(newt[i]);
            }

            Console.SetCursorPosition(0, prev_y);
            Console.ForegroundColor = ConsoleColor.Gray;
            Console.BackgroundColor = ConsoleColor.Black;
        }

        public void End()
        {
            this.percent = 1.0f;
            this.title += " COMPLETED";
            this.Draw(true);
            Console.Write("\n");
        }

        public void Error()
        {
            this.title += " FAILED";
            this.Draw(true, true);
            Console.Write("\n");
        }

        public ProgressViewer(string title)
        {
            this.title = title;

            this.console_y = Console.CursorTop;

            this.Draw();
        }
    }
}