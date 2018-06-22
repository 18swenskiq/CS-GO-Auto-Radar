using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace MCDV_Processor
{
    class Debug
    {

        private static bool isProgressBar = false;
        private static string headerRef = "";
        private static string currmessage = "";

        enum DebugLevel
        {
            low,
            normal,
            high
        }

        public static void Log(string msg, params object[] args) { printMsg(string.Format(msg, args), ConsoleColor.Gray, DebugLevel.high, "log"); }
        public static void Info(string msg, params object[] args) { printMsg(string.Format(msg, args), ConsoleColor.White, DebugLevel.normal, "INFO"); }
        public static void Success(string msg, params object[] args) { printMsg(string.Format(msg, args), ConsoleColor.Cyan, DebugLevel.low, "Success"); }
        public static void Error(string msg, params object[] args) { printMsg(string.Format(msg, args), ConsoleColor.Red, DebugLevel.low, "Error"); }
        public static void Warn(string msg, params object[] args) { printMsg(string.Format(msg, args), ConsoleColor.DarkCyan, DebugLevel.normal, "Warn"); }
        public static void Headings(string msg, params object[] args) { printMsg(string.Format(msg, args), ConsoleColor.White, DebugLevel.normal, "", false); }

        public static void Blue(string msg, params object[] args) { printMsg(string.Format(msg, args), ConsoleColor.DarkGray, DebugLevel.normal, "", false, true); }
        public static void White(string msg, params object[] args) { printMsg(string.Format(msg, args), ConsoleColor.White, DebugLevel.normal, "", false, true); }

        public static void progressBar(string header, int percent)
        {
            isProgressBar = true;
            headerRef = header;
            Console.Write("\n");

            updateProgressBar(percent);
        }

        public static void updateProgressBar(int percent)
        {
            if (isProgressBar)
            {
                Console.Write("\r");
                for (int i = 0; i < 100; i++) //Clears everything so stuff doesnt hang around
                    Console.Write(" ");


                Console.ForegroundColor = ConsoleColor.White;
                Console.Write("\r{0} [", headerRef);

                int bars = Math.Max(0, Math.Min(percent / 4, 100)); //hacky clamp

                Console.ForegroundColor = ConsoleColor.Cyan;

                for (int i = 0; i < bars; i++)
                    Console.Write("#");
                for (int i = 0; i < 25 - bars; i++)
                    Console.Write(" ");

                Console.ForegroundColor = ConsoleColor.White;
                Console.Write("] {0}%", percent);

                Console.Write(" -> {0}", currmessage);
            }
        }

        public static void exitProgressBar()
        {
            updateProgressBar(100);

            isProgressBar = false;
            Console.WriteLine("  COMPLETE");
        }

        private static void printMsg(string message, ConsoleColor c, DebugLevel lvl = DebugLevel.low, string prefix = "", bool usedate = true, bool bluepost = false)
        {

            if (bluepost)
            {
                Console.ForegroundColor = c;
                Console.Write(message);
                Console.ForegroundColor = ConsoleColor.Gray;
                return;
            }

            if (!isProgressBar)
            {
                if (lvl <= DebugLevel.high)
                {
                    Console.ForegroundColor = c;
                    if (usedate)
                        Console.Write("[" + DateTime.Now.ToShortTimeString() + " | " + prefix + "] ");
                    Console.Write(message + "\n");
                    Console.ForegroundColor = ConsoleColor.Gray;
                }
            }
            else
            {
                currmessage = message;
            }
        }
    }
}