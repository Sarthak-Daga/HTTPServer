document.addEventListener('DOMContentLoaded', function () {
  const navLinks = document.querySelectorAll('nav a');
  navLinks.forEach(link => {
    link.addEventListener('click', function () {
      alert('Navigating to ' + this.textContent);
    });
  });
});
