#include <jw.h>
#include <jw_quickjs.h>
#include <metamodel_gen.h>

struct UserData {
	struct Type *type;
	void *data;
	property_iterator_fn cb;
};

static int
property_iterator_cb(
		struct Jw *jw, struct JwVal *property, int index, void *data) {
	int rv = 0;
	struct UserData *user_data = data;
	char *field_name = NULL;
	struct JwVal field_type = {0};
	
	rv = jw_obj_get_str(jw, property, "name", &field_name, NULL);
	rv = jw_obj_get(jw, property, "type", &field_type);
	if (!jw_is_obj(jw, &field_type)) {
		jw_cleanup(jw, &field_type);
		rv = jw_dup(jw, &field_type, property);
	}
	if (rv < 0) {
		goto out;
	}

	rv = user_data->cb(user_data->type, field_name, &field_type, user_data->data);
	if (rv < 0) {
		goto out;
	}

out:
	jw_cleanup(jw, &field_type);
	free(field_name);
	return 0;
}

int
property_iterator(
		struct Type *type, const char *property_field,
		property_iterator_fn cb, void *data) {
	int rv = 0;
	struct JwVal property_array = {0};
	struct UserData user_data = {
			.type = type,
			.data = data,
			.cb = cb,
	};

	rv = jw_obj_get(
			type->generator->jw, &type->definition, property_field,
			&property_array);
	if (rv < 0) {
		goto out;
	}

	rv = jw_arr_foreach(
			type->generator->jw, &property_array, property_iterator_cb,
			&user_data);
	if (rv < 0) {
		goto out;
	}

out:
	jw_cleanup(type->generator->jw, &property_array);
	return 0;
}
