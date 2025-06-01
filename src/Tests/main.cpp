#include "gtest/gtest.h"

#include <gtk/gtk.h>

#include "CompareFiles.hpp"

std::string g_DataDir{};
bool g_CleanArtifacts{true};

static int OnCommandLine(GtkApplication *app, GApplicationCommandLine *cmdline)
{
    int argc;
    char **argv = g_application_command_line_get_arguments(cmdline, &argc);

    for (auto i = 0; i < argc; ++i)
    {
        if (strncmp(argv[i], "--data_dir=", 11) == 0)
        {
            g_DataDir = argv[i];
            g_DataDir = g_DataDir.substr(11); // Remove the "--data_dir=" prefix
        }

        if (strncmp(argv[i], "--leave_artifacts", 11) == 0)
        {
            g_CleanArtifacts = false;
        }
    }

    testing::InitGoogleTest(&argc, argv);
    g_strfreev(argv);

    return RUN_ALL_TESTS();
}

int main(int argc, char **argv)
{
    auto app = gtk_application_new("com.patrickfournier.gorfector-tests", G_APPLICATION_HANDLES_COMMAND_LINE);
    g_signal_connect(app, "command-line", G_CALLBACK(OnCommandLine), nullptr);
    return g_application_run(G_APPLICATION(app), argc, argv);
}
