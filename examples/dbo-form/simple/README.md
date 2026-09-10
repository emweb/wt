Dbo Form simple example
-----------------------

This example shows a minimal `Wt::Form::Dbo` setup. The source comments walk
through mapping a persistent object, creating its form, and saving it.

How to run
----------

Build Wt with examples, Wt::Dbo, the SQLite backend, and the HTTP connector
enabled. From the repository root:

```sh
cmake -S . -B build -DBUILD_EXAMPLES=ON -DENABLE_LIBWTDBO=ON \
  -DENABLE_SQLITE=ON -DCONNECTOR_HTTP=ON
cmake --build build --target dbo-form-simple.wt -j2
```

From `examples/dbo-form/simple`, run:

```sh
../../../build/examples/dbo-form/simple/dbo-form-simple.wt \
  --docroot . \
  --approot approot \
  --http-listen 0.0.0.0:8080 \
  --resources-dir=../../../resources
```

Open `http://localhost:8080/`. The `approot` directory must be writable because
the example creates `approot/dbo-form-simple.db` there.

Fill in the fields and press Save to insert a new contact and display `Saved.`.
Enter nonnumeric text in Age or Balance and press Save to see the default
validation message and `Please correct the highlighted fields.`. Invalid
values are not saved. Email is a plain string with no email-specific validator.
