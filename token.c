#include <srcml.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {

    const char* s = "int main() {\n return 0; }";

    struct srcml_archive* archive = srcml_archive_create();

    srcml_archive_write_open_filename(archive, "rip.xml");

    {
        struct srcml_unit* unit = srcml_unit_create(archive);
        srcml_unit_set_language(unit, SRCML_LANGUAGE_C);
        srcml_unit_parse_memory(unit, s, strlen(s));
        const char *result = srcml_unit_get_srcml(unit);
        printf("%s\n", result);
        srcml_archive_write_unit(archive, unit);
        srcml_unit_free(unit);
    }

    srcml_archive_close(archive);
    srcml_archive_free(archive);

    return 0;
}
