-- cannot do on SQLite3:
-- alter table "user" drop column "token";
alter table "user" add column "password_method" text not null default "crypt";
alter table "user" add column "password_salt" text not null default "";
alter table "user" add column "failed_login_attempts" integer not null default 0;
alter table "user" add column "last_login_attempt" text;
alter table "user" add column "oauth_id" text not null default "";
alter table "user" add column "oauth_provider" text not null default "";

create table "token" (
  "id" integer primary key autoincrement,
  "version" integer not null,
  "value" text not null,
  "expires" text,
  "user_id" bigint,
  constraint "fk_token_user" foreign key ("user_id") references "user" ("id")
)
